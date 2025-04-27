const columnCheckboxes = document.querySelectorAll('input[type="checkbox"][name="columns"]');
const queryInput = document.getElementById('query-input');
const orderBySelect = document.getElementById('order-by');
const limitInput = document.getElementById('limit-input');
//const orderDirectionRadios = document.querySelectorAll('input[type="radio"][name="order-direction"]');
const orderByDescCheckbox = document.getElementById('orderByDesc');
const queryButton = document.getElementById('query-button');
const queryResultDiv = document.getElementById('query-result');

const musicPath = 'file/Maroon 5-01-Moves Like Jagger (Feat. Christina Aguilera) (Studio Recording From The Voice Performance)-128.mp3'; // 실제 경로로 변경

// <audio> 태그 사용
//const audio = new Audio(musicPath);
// audio.play();

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

async function playMp3(filename) {
  try {
    const audio = new Audio(filename)
    audio.play();
    console.log('MP3 재생 성공:', filename);
    audio.addEventListener('ended', () => {
    console.log('재생 완료');
  });
  } catch (error) {
    console.error('MP3 재생 오류:', error);
    // 사용자에게 에러 알림
  }
}

document.querySelectorAll('.play-mp3').forEach(link => {
  link.addEventListener('click', (e) => {
    e.preventDefault();
    const filename = e.target.getAttribute('data-filename');
    //console.log('->',filename);
    playMp3(filename);

  });
});

// 예시: 버튼 클릭 이벤트 등에서 호출
document.getElementById('play-button').addEventListener('click', () => {
  const filename = 'example.mp3'; // 실제 파일 이름으로 대체
  playMp3(filename);
  const audio = new Audio(filename);
    
  audio.play();
});
