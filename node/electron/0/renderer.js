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

// 엔터 키 처리 (줄바꿈 방지)
queryResultDiv.addEventListener('keydown', (e) => {
  if (e.target.hasAttribute('contenteditable')) {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      e.target.blur();
    }
  }
});

// 포커스 아웃 시 업데이트 확인
queryResultDiv.addEventListener('focusout', (e) => {
  if (e.target.hasAttribute('contenteditable')) {
    if (e.target.classList.contains('modified')) {
      //alert("수정된 내용을 저장하세요");
    }
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
      output += `<tr data-id="${row.id}">`; // 여기에 data-id 설정
      output += `<td style="display:none;">${row.id}</td>`; // id를 숨김
      selectedColumns.forEach(column => {
        const displayValue = row[column] ?? '';
        if (column === 'cover_image' && row[column]) {
          // Base64 이미지 데이터를 img 태그로 변환
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
    // MP3 재생 버튼 이벤트 리스너 등록
    document.querySelectorAll('.play-mp3').forEach(link => {
      link.addEventListener('click', (e) => {
        e.preventDefault();
        const filename = e.target.getAttribute('data-filename');
        window.electronAPI.playMp3(filename);
      });
    });
    console.log(output);
  } else if (results && results.length === 0) {
    queryResultDiv.textContent = '조회 결과가 없습니다.';
  } else {
    queryResultDiv.textContent = '데이터베이스 조회 실패.';
  }
}

async function playMp3(filename) {
  try {
    const audio = new Audio(filename);
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

