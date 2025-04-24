const queryInput = document.getElementById('query-input');
const queryButton = document.getElementById('query-button');
const queryResultDiv = document.getElementById('query-result');

queryButton.addEventListener('click', async () => {
  const whereClause = queryInput.value;
  const results = await window.electronAPI.queryDatabase(whereClause);

  if (results && results.length > 0) {
    let output = '<table><thead><tr>';
    for (const key in results[0]) {
      output += `<th>${key}</th>`;
    }
    output += '</tr></thead><tbody>';
    results.forEach(row => {
      output += '<tr>';
      for (const key in row) {
        output += `<td>${row[key]}</td>`;
      }
      output += '</tr>';
    });
    output += '</tbody></table>';
    queryResultDiv.innerHTML = output;
  } else if (results && results.length === 0) {
    queryResultDiv.textContent = '조회 결과가 없습니다.';
  } else {
    queryResultDiv.textContent = '데이터베이스 조회 실패.';
  }
});
