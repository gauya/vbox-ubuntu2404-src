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


document.addEventListener('keydown', (e) => {
  if (e.ctrlKey && e.key === 'Enter') {
    if(updateButton.style.display === 'inline-block') {
      updateButton.click();  
    }
  }
});

// 전역 변수로 현재 수정 중인 셀 추적
let currentEditingCell = null;
let originalValue = '';


queryButton.insertAdjacentElement('afterend', updateButton);

// 수정된 셀 추적을 위한 Set
const modifiedCells = new Set();

// 수정 감지 및 버튼 표시
queryResultDiv.addEventListener('input', (e) => {
  if (e.target.hasAttribute('contenteditable')) {
    const original = e.target.dataset.original;
    const current = e.target.textContent.trim();

    if (original !== current) {
      modifiedCells.add(e.target);
      e.target.classList.add('modified');
    } else {
      modifiedCells.delete(e.target);
      e.target.classList.remove('modified');
    }

    updateButton.style.display = modifiedCells.size > 0 ? 'inline-block' : 'none';
  }
});

// 키보드 이벤트 핸들러 추가
queryResultDiv.addEventListener('keydown', (e) => {
  if (!currentEditingCell && e.target.hasAttribute('contenteditable')) {
    currentEditingCell = e.target;
    originalValue = currentEditingCell.textContent.trim();
  }

  // 수정 모드 중 방향키 처리
  if (currentEditingCell) {
    const key = e.key;
    const isModified = currentEditingCell.textContent.trim() !== originalValue;
    const shouldSave = isModified &&
                     (key === 'ArrowRight' || key === 'ArrowLeft' ||
                      key === 'ArrowUp' || key === 'ArrowDown');

    if (shouldSave) {
      // 값이 변경된 경우 저장 처리
      currentEditingCell.dataset.original = currentEditingCell.textContent.trim();
      modifiedCells.add(currentEditingCell);
      currentEditingCell.classList.add('modified');
      updateButton.style.display = 'inline-block';
    } else if (!isModified &&
              (key === 'ArrowRight' || key === 'ArrowLeft' ||
               key === 'ArrowUp' || key === 'ArrowDown')) {
      // 값이 변경되지 않은 경우 원래 값 복원
      currentEditingCell.textContent = originalValue;
    }

    // 방향키에 따라 이동 처리
    if (key === 'ArrowRight') {
      e.preventDefault();
      moveToAdjacentCell(1, 0); // 오른쪽 셀
    } else if (key === 'ArrowLeft') {
      e.preventDefault();
      moveToAdjacentCell(-1, 0); // 왼쪽 셀
    } else if (key === 'ArrowDown') {
      e.preventDefault();
      moveToAdjacentCell(0, 1); // 아래쪽 셀
    } else if (key === 'ArrowUp') {
      e.preventDefault();
      moveToAdjacentCell(0, -1); // 위쪽 셀
    } else if (key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      if (e.target.hasAttribute('contenteditable')) {
        moveToAdjacentCell(0, 1); // 엔터키 = 아래쪽 셀
      }
    }
  }
});

// 인접 셀로 이동하는 함수
function moveToAdjacentCell(xDiff, yDiff) {
  if (!currentEditingCell) return;

  const row = currentEditingCell.closest('tr');
  const cells = Array.from(row.querySelectorAll('[contenteditable]'));
  const currentIndex = cells.indexOf(currentEditingCell);
  const nextCellIndex = currentIndex + xDiff;

  // 같은 행 내 좌우 이동
  if (xDiff !== 0 && nextCellIndex >= 0 && nextCellIndex < cells.length) {
    currentEditingCell.blur();
    const nextCell = cells[nextCellIndex];
    nextCell.focus();
    selectCellContent(nextCell);
    currentEditingCell = nextCell;
    originalValue = nextCell.textContent.trim();
  }
  // 다른 행으로 상하 이동
  else if (yDiff !== 0) {
    const nextRow = yDiff > 0 ? row.nextElementSibling : row.previousElementSibling;
    if (nextRow) {
      currentEditingCell.blur();
      const nextRowCells = Array.from(nextRow.querySelectorAll('[contenteditable]'));
      const targetCell = nextRowCells[Math.min(currentIndex, nextRowCells.length - 1)];
      targetCell.focus();
      selectCellContent(targetCell);
      currentEditingCell = targetCell;
      originalValue = targetCell.textContent.trim();

      // 행 하이라이트 업데이트
      document.querySelectorAll('#query-result tr').forEach(r => {
        r.classList.remove('focused');
      });
      nextRow.classList.add('focused');
    }
  }
}

// 셀 내용 전체 선택 함수
function selectCellContent(cell) {
  const range = document.createRange();
  range.selectNodeContents(cell);
  const selection = window.getSelection();
  selection.removeAllRanges();
  selection.addRange(range);
}

// 셀 포커스 아웃 시 처리
queryResultDiv.addEventListener('focusout', (e) => {
  if (e.target.hasAttribute('contenteditable')) {
    if (e.target.textContent.trim() !== originalValue) {
      e.target.classList.add('modified');
      modifiedCells.add(e.target);
      updateButton.style.display = 'inline-block';
    }
    currentEditingCell = null;
  }
});

// 업데이트 버튼 클릭 핸들러
updateButton.addEventListener('click', async () => {
  console.log('Sending updates start');

  if (modifiedCells.size === 0) {
    console.log('modifiedCells.size === 0');
    return;
  }
  const updates = [];
  modifiedCells.forEach(cell => {
    const row = cell.closest('tr');
    const id = row.dataset.id; // 수정됨: data-id 속성 사용
    const column = cell.dataset.column;
    const value = cell.textContent.trim();

    console.log('update push (id,column,value) :', id, column, value);
    updates.push({ id, column, value });
  });

  console.log('Preparing to send updates:', updates); // 디버깅 로그

  try {
    console.log('Calling window.electronAPI.updateDatabase');
    const result = await window.electronAPI.updateDatabase(updates);
    console.log('Received response:', result);
    if (result && result.success) {
      // 성공 시 원래 값 업데이트 및 스타일 제거
      modifiedCells.forEach(cell => {
        cell.dataset.original = cell.textContent.trim();
        cell.classList.remove('modified');
      });
      modifiedCells.clear();
      updateButton.style.display = 'none';
    } else {
      throw new Error(result?.error || '업데이트 실패');
    }
  } catch (error) {
    console.error('Update error:', error);
    alert(`저장 실패: ${error.message}`);
  }
});

queryButton.addEventListener('click', async () => {
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

  modifiedCells.clear();
  updateButton.style.display = 'none';

  const queryOptions = {
    columns: ['id', ...selectedColumns],
    where: whereClause,
    orderBy: orderByColumn,
    orderDirection: orderDirection,
    limit: limitClause,
  };

  try {
    const results = await window.electronAPI.queryDatabase(queryOptions);
    displayResults(results, selectedColumns);
  } catch (error) {
    console.error('Query error:', error);
    queryResultDiv.textContent = '데이터베이스 조회 실패.';
  }
});

function displayResults(results, selectedColumns) {
  if (results && results.length > 0) {
    let output = '<table><thead><tr>';
    selectedColumns.forEach(column => {
      output += `<th>${column}</th>`;
    });
    output += '</tr></thead><tbody>';

    results.forEach(row => {
      output += `<tr data-id="${row.id}" tabindex="0">`; // tabindex 추가
      output += `<td style="display:none;">${row.id}</td>`;

      selectedColumns.forEach(column => {
        const displayValue = row[column] ?? '';
        if (column === 'cover_image' && row[column]) {
          output += `<td><img src="data:image/jpeg;base64,${row[column]}" style="max-width: 100px; max-height: 100px;"></td>`;
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

    // 행 포커스 이벤트 핸들러 추가
    document.querySelectorAll('#query-result tr[data-id]').forEach(row => {
      // 마우스 클릭 시
      row.addEventListener('click', () => {
        document.querySelectorAll('#query-result tr').forEach(r => {
          r.classList.remove('focused');
        });
        row.classList.add('focused');
      });

      // 키보드 포커스 시 (탭 이동)
      row.addEventListener('focus', () => {
        document.querySelectorAll('#query-result tr').forEach(r => {
          r.classList.remove('focused');
        });
        row.classList.add('focused');
      });
    });

    // MP3 재생 버튼 이벤트 리스너
    document.querySelectorAll('.play-mp3').forEach(link => {
      link.addEventListener('click', (e) => {
        e.preventDefault();
        const filename = e.target.getAttribute('data-filename');
        window.electronAPI.playMp3(filename);

        // 재생 버튼 클릭 시 해당 행 강조
        const row = e.target.closest('tr');
        document.querySelectorAll('#query-result tr').forEach(r => {
          r.classList.remove('focused');
        });
        row.classList.add('focused');
      });
    });
  }
};

async function playMp3(filename) {
  try {
    const audio = new Audio(filename);
    audio.volume = 1.0; // 최대 음량 설정
    audio.play();
    console.log('MP3 재생 성공 renderer :', filename);
    audio.addEventListener('ended', () => {
      console.log('재생 완료');
    });
  } catch (error) {
    console.error('MP3 재생 오류:', error);
    // 사용자에게 에러 알림
  }
}

