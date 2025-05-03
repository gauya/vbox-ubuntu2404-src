const columnCheckboxes = document.querySelectorAll('input[type="checkbox"][name="columns"]');
const queryInput = document.getElementById('query-input');
const orderBySelect = document.getElementById('order-by');
const limitInput = document.getElementById('limit-input');
const orderByDescCheckbox = document.getElementById('orderByDesc');
const queryButton = document.getElementById('query-button');
const queryResultDiv = document.getElementById('query-result');

// 변경사항 저장 버튼 생성
const updateButton = document.createElement('button');
updateButton.id = 'update-button';
updateButton.textContent = '변경사항 저장';
updateButton.style.display = 'none';
updateButton.style.margin = '10px 0';
updateButton.style.padding = '8px 16px';
updateButton.style.backgroundColor = '#4CAF50';
updateButton.style.color = 'white';
updateButton.style.border = 'none';
updateButton.style.borderRadius = '4px';
updateButton.style.cursor = 'pointer';

// 전역 상태 관리
const appState = {
  currentCell: {
    row: null,
    cell: null,
    isEditMode: false,
    originalValue: ''
  },
  modifiedCells: new Set()
};

// 초기화 함수
function initialize() {
  queryButton.insertAdjacentElement('afterend', updateButton);
  setupEventListeners();
}

// 이벤트 리스너 설정
function setupEventListeners() {
  // Ctrl+Enter로 저장
  document.addEventListener('keydown', (e) => {
    if (e.ctrlKey && e.key === 'Enter' && updateButton.style.display === 'inline-block') {
      updateButton.click();
    }
  });

  // 변경 감지
  queryResultDiv.addEventListener('input', handleCellInput);
  
  // 테이블 이벤트
  const table = document.querySelector('#query-result table');
  if (table) {
    table.addEventListener('click', handleTableClick);
    table.addEventListener('keydown', handleTableKeyDown);
    table.addEventListener('scroll', handleTableScroll);
  }

  // 업데이트 버튼
  updateButton.addEventListener('click', handleUpdate);
  queryButton.addEventListener('click', handleQuery);
}

// 셀 입력 처리
function handleCellInput(e) {
  if (e.target.hasAttribute('contenteditable')) {
    const original = e.target.dataset.original;
    const current = e.target.textContent.trim();

    if (original !== current) {
      appState.modifiedCells.add(e.target);
      e.target.classList.add('modified');
    } else {
      appState.modifiedCells.delete(e.target);
      e.target.classList.remove('modified');
    }

    updateButton.style.display = appState.modifiedCells.size > 0 ? 'inline-block' : 'none';
  }
}

// 테이블 클릭 처리
function handleTableClick(e) {
  const cell = e.target.closest('td[contenteditable]');
  if (!cell) return;

  const row = cell.closest('tr');
  setFocus(row, cell, true);
}

// 테이블 키보드 처리
function handleTableKeyDown(e) {
  if (!appState.currentCell.cell) return;

  const {key} = e;
  const isModified = appState.currentCell.cell.textContent.trim() !== appState.currentCell.originalValue;

  switch(key) {
    case 'Escape':
      e.preventDefault();
      exitEditMode();
      break;
      
    case 'Enter':
      if (!e.shiftKey) {
        e.preventDefault();
        if (appState.currentCell.isEditMode) {
          exitEditMode();
          moveFocus('down');
        } else {
          enterEditMode();
        }
      }
      break;
      
    case 'ArrowUp':
    case 'ArrowDown':
    case 'ArrowLeft':
    case 'ArrowRight':
      e.preventDefault();
      if (appState.currentCell.isEditMode && isModified) {
        saveCurrentCell();
      }
      moveFocus(key.replace('Arrow', '').toLowerCase());
      break;
  }
}

// 테이블 스크롤 처리
function handleTableScroll() {
  if (appState.currentCell.row && !isElementInViewport(appState.currentCell.row)) {
    appState.currentCell.row.scrollIntoView({behavior: 'smooth', block: 'nearest'});
  }
}

async function handleQuery() {
  const selectedColumns = Array.from(columnCheckboxes)
    .filter(checkbox => checkbox.checked)
    .map(checkbox => checkbox.value);

  const whereClause = queryInput.value.trim();
  const orderByColumn = orderBySelect.value;
  const limitClause = limitInput.value.trim();
  const orderDirection = orderByDescCheckbox.checked ? 'DESC' : 'ASC';

  if (selectedColumns.length === 0) {
    queryResultDiv.textContent = '출력할 컬럼을 하나 이상 선택해주세요.';
    return;
  }

  appState.modifiedCells.clear();
  updateButton.style.display = 'none';

  try {
    const results = await window.electronAPI.queryDatabase({
      columns: ['id', ...selectedColumns],
      where: whereClause,
      orderBy: orderByColumn,
      orderDirection: orderDirection,
      limit: limitClause
    });

    displayResults(results, selectedColumns);
  } catch (error) {
    console.error('Query error:', error);
    queryResultDiv.innerHTML = `<div class="error">데이터베이스 조회 실패: ${error.message}</div>`;
  }
}

// 포커스 설정
function setFocus(row, cell, shouldEnterEditMode = false) {
  // 기존 포커스 제거
  if (appState.currentCell.row) {
    appState.currentCell.row.classList.remove('focus-row');
  }
  if (appState.currentCell.cell) {
    appState.currentCell.cell.classList.remove('focus-cell', 'edit-mode');
    if (appState.currentCell.isEditMode) {
      appState.currentCell.cell.blur();
    }
  }

  // 새로운 포커스 설정
  appState.currentCell.row = row;
  appState.currentCell.cell = cell;
  
  if (row) {
    row.classList.add('focus-row');
    if (!isElementInViewport(row)) {
      row.scrollIntoView({behavior: 'smooth', block: 'nearest'});
    }
  }
  
  if (cell) {
    cell.classList.add('focus-cell');
    if (shouldEnterEditMode) {
      enterEditMode();
    }
  }
}

// 수정 모드 진입
function enterEditMode() {
  if (!appState.currentCell.cell) return;
  
  appState.currentCell.isEditMode = true;
  appState.currentCell.originalValue = appState.currentCell.cell.textContent.trim();
  
  appState.currentCell.cell.classList.add('edit-mode');
  appState.currentCell.cell.focus();
  selectCellContent(appState.currentCell.cell);
}

// 노멀 모드 진입
function exitEditMode() {
  if (!appState.currentCell.cell) return;
  
  appState.currentCell.isEditMode = false;
  appState.currentCell.cell.classList.remove('edit-mode');
  appState.currentCell.cell.blur();
}

// 포커스 이동
function moveFocus(direction) {
  if (!appState.currentCell.row || !appState.currentCell.cell) return;

  const rows = Array.from(document.querySelectorAll('#query-result tr[data-id]'));
  const currentRowIdx = rows.indexOf(appState.currentCell.row);
  const cells = Array.from(appState.currentCell.row.querySelectorAll('td[contenteditable]'));
  const currentCellIdx = cells.indexOf(appState.currentCell.cell);

  let newRow = appState.currentCell.row;
  let newCell = appState.currentCell.cell;

  switch(direction) {
    case 'up':
      if (currentRowIdx > 0) {
        newRow = rows[currentRowIdx - 1];
        newCell = newRow.querySelectorAll('td[contenteditable]')[currentCellIdx];
      }
      break;
    case 'down':
      if (currentRowIdx < rows.length - 1) {
        newRow = rows[currentRowIdx + 1];
        newCell = newRow.querySelectorAll('td[contenteditable]')[currentCellIdx];
      }
      break;
    case 'left':
      if (currentCellIdx > 0) {
        newCell = cells[currentCellIdx - 1];
      }
      break;
    case 'right':
      if (currentCellIdx < cells.length - 1) {
        newCell = cells[currentCellIdx + 1];
      }
      break;
  }

  setFocus(newRow, newCell, appState.currentCell.isEditMode);
}

// 셀 내용 선택
function selectCellContent(cell) {
  const range = document.createRange();
  range.selectNodeContents(cell);
  const selection = window.getSelection();
  selection.removeAllRanges();
  selection.addRange(range);
}

// 요소가 뷰포트 안에 있는지 확인
function isElementInViewport(el) {
  const rect = el.getBoundingClientRect();
  return (
    rect.top >= 0 &&
    rect.left >= 0 &&
    rect.bottom <= (window.innerHeight || document.documentElement.clientHeight) &&
    rect.right <= (window.innerWidth || document.documentElement.clientWidth)
  );
}

function displayResults(results, selectedColumns) {
  if (!results || results.length === 0) {
    queryResultDiv.innerHTML = '<div class="no-data">조회 결과가 없습니다.</div>';
    return;
  }

  let output = '<table><thead><tr>';
  selectedColumns.forEach(column => {
    output += `<th>${column}</th>`;
  });
  output += '</tr></thead><tbody>';

  results.forEach(row => {
    output += `<tr data-id="${row.id}" tabindex="0">`;
    output += `<td style="display:none;">${row.id}</td>`;

    selectedColumns.forEach(column => {
      const displayValue = row[column] ?? '';
      if (column === 'cover_image' && row[column]) {
        output += `<td><img src="data:image/jpeg;base64,${row[column]}" class="album-cover"></td>`;
      } else if (column === 'file_name' && row[column]) {
        output += `<td><a href="#" class="play-mp3" data-filename="${row[column]}">${displayValue}</a></td>`;
      } else {
        output += `<td contenteditable="true" data-column="${column}" data-original="${displayValue}">${displayValue}</td>`;
      }
    });
    output += '</tr>';
  });

  output += '</tbody></table>';
  queryResultDiv.innerHTML = output;

  // 새로 생성된 요소에 이벤트 리스너 재등록
  document.querySelectorAll('#query-result tr[data-id]').forEach(row => {
    row.addEventListener('click', (e) => {
      const cell = e.target.closest('td[contenteditable]');
      if (cell) {
        const row = cell.closest('tr');
        setFocus(row, cell, true);
      }
    });
  });

  document.querySelectorAll('.play-mp3').forEach(link => {
    link.addEventListener('click', (e) => {
      e.preventDefault();
      const filename = e.target.getAttribute('data-filename');
      window.electronAPI.playMp3(filename);
      setFocus(e.target.closest('tr'), null, false);
    });
  });
}

// 초기화 함수 수정
function initialize() {
  queryButton.insertAdjacentElement('afterend', updateButton);

  // 컨트롤 요소에 기본값 설정
  columnCheckboxes.forEach(checkbox => {
    checkbox.checked = true; // 기본적으로 모든 컬럼 선택
  });
  limitInput.value = '100'; // 기본 조회 제한 설정

  setupEventListeners();
}

async function handleUpdate() {
  if (appState.modifiedCells.size === 0) return;

  const updates = [];
  appState.modifiedCells.forEach(cell => {
    const row = cell.closest('tr');
    updates.push({
      id: row.dataset.id,
      column: cell.dataset.column,
      value: cell.textContent.trim()
    });
  });

  try {
    const result = await window.electronAPI.updateDatabase(updates);
    if (result?.success) {
      appState.modifiedCells.forEach(cell => {
        cell.dataset.original = cell.textContent.trim();
        cell.classList.remove('modified');
      });
      appState.modifiedCells.clear();
      updateButton.style.display = 'none';
    } else {
      throw new Error(result?.error || '업데이트 실패');
    }
  } catch (error) {
    console.error('Update error:', error);
    alert(`저장 실패: ${error.message}`);
  }
}

// DOM 로드 완료 시 초기화
document.addEventListener('DOMContentLoaded', initialize);

