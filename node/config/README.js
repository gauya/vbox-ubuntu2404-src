// 환경별로 설정파일을 따로 쓰기
const env = process.env.NODE_ENV || 'development';
const configManager = new ConfigManager(`./config.${env}.ini`);


// 설정변경을 감지하여 리로드
fs.watch(configPath, (eventType, filename) => {
  if (eventType === 'change') {
    configManager.reload();
    console.log('설정 파일이 변경되어 리로드되었습니다.');
  }
});



// TypeScript
interface AppConfig {
  database: {
    host: string;
    user: string;
    password: string;
    // ...
  };
  server: {
    port: number;
    // ...
  };
}

const config = ini.parse(fs.readFileSync('./config.ini', 'utf-8')) as AppConfig;
