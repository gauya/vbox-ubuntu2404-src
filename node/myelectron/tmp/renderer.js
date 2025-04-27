const columnCheckboxes = document.querySelectorAll('input[type="checkbox"][name="columns"]');
const queryInput = document.getElementById('query-input');
const orderBySelect = document.getElementById('order-by');
const limitInput = document.getElementById('limit-input');
//const orderDirectionRadios = document.querySelectorAll('input[type="radio"][name="order-direction"]');
const orderByDescCheckbox = document.getElementById('orderByDesc');
const queryButton = document.getElementById('query-button');
const queryResultDiv = document.getElementById('query-result');

queryButton.addEventListener('click', async () => {
  const selectedColumns = Array.from(columnCheckboxes)
    .filter(checkbox => checkbox.checked)
    .map(checkbox => checkbox.value);

  const whereClause = queryInput.value.trim();
  const orderByColumn = orderBySelect.value;
  const limitClause = limitInput.value.trim();
  const orderDirection = orderByDescCheckbox.checked ? 'DESC' : 'ASC';
  
  /*
  let orderDirection = '';
  orderDirectionRadios.forEach(radio => {
    if (radio.checked) {
      orderDirection = radio.value;
    }
  });
*/
  if (selectedColumns.length === 0) {
    queryResultDiv.textContent = '출력할 컬럼을 하나 이상 선택해주세요.';
    return;
  }

  const queryOptions = {
    columns: selectedColumns,
    where: whereClause,
    orderBy: orderByColumn,
    orderDirection: orderDirection,
    limit: limitClause, 
  };

  const results = await window.electronAPI.queryDatabase(queryOptions);

  if (results && results.length > 0) {
    let output = '<table><thead><tr>';
    selectedColumns.forEach(column => {
      output += `<th>${column}</th>`;
    });
    output += '</tr></thead><tbody>';
    results.forEach(row => {
      output += '<tr>';
      selectedColumns.forEach(column => {
      if (column === 'cover_image' && row[column]) {
        // Base64 이미지 데이터를 img 태그로 변환
        output += `<td><img src="data:image/jpeg;base64,${row[column]}" style="max-width: 100px; max-height: 100px;"></td>`;
      }else if (column === 'file_name' && row[column]) {
          output += `<td><a href="#" class="play-mp3" data-filename="${row[column]}">${row[column]}</a></td>`;
    } else {
        output += `<td>${row[column] || ''}</td>`;
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
});
/*
async function playMp3(filename) {
  try {
    const mp3Path = '/db/mp3/file/' + filename; // main.js에서 처리하도록 경로만 전달
    const fileExists = await window.electronAPI.checkFileExists(mp3Path);

    if (!fileExists) {
      console.error('파일이 존재하지 않습니다:', mp3Path);
      // 사용자에게 알림 (예: alert, UI 업데이트)
      return;
    }

    await window.electronAPI.openFile(mp3Path); // 파일 열기 요청
    console.log('MP3 재생 성공:', mp3Path);

  } catch (error) {
    console.error('MP3 재생 오류:', error);
    // 사용자에게 에러 알림
  }
}

// 예시: 버튼 클릭 이벤트 등에서 호출
document.getElementById('play-button').addEventListener('click', () => {
  const filename = 'example.mp3'; // 실제 파일 이름으로 대체
  playMp3(filename);
});
*/
async function playMp3(filename) {
  try {
    const mp3Path = '/db/mp3/file/' + filename;
    // const fileExists = await window.electronAPI.checkFileExists(mp3Path); // 파일 존재 여부 확인은 이미 처리되었다고 가정

    // if (!fileExists) {
    //   console.error('파일이 존재하지 않습니다:', mp3Path);
    //   // 사용자에게 알림 (예: alert, UI 업데이트)
    //   return;
    // }

    await window.electronAPI.openFile(mp3Path); // main.js에 파일 열기 요청
    console.log('MP3 재생 성공:', mp3Path);

  } catch (error) {
    console.error('MP3 재생 오류:', error);
    // 사용자에게 에러 알림
  }
}

// 예시: 버튼 클릭 이벤트 등에서 호출
document.getElementById('play-button').addEventListener('click', () => {
  const filename = 'example.mp3'; // 실제 파일 이름으로 대체
  playMp3(filename);
});
