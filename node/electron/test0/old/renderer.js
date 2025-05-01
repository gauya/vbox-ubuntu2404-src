console.log('Node.js 버전:', window.versions.node());
console.log('Chrome 버전:', window.versions.chrome());
console.log('Electron 버전:', window.versions.electron());

const information = document.getElementById('info');
information.innerText = `This app is using Chrome (v${process.versions.chrome}), Node.js (v${process.versions.node}), and Electron (v${process.versions.electron})`;
