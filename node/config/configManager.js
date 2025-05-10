const fs = require('fs');
const path = require('path');
const ini = require('ini');

class ConfigManager {
  constructor(configPath) {
    this.configPath = path.resolve(configPath);
    this.config = this._loadConfig();
  }

  // 설정 파일 로드
  _loadConfig() {
    try {
      const fileContent = fs.readFileSync(this.configPath, 'utf-8');
      return ini.parse(fileContent);
    } catch (error) {
      console.error('설정 파일 로드 실패:', error);
      return {};
    }
  }

  // 설정 값 가져오기
  get(section, key) {
    return this.config[section]?.[key];
  }

  // 전체 섹션 가져오기
  getSection(section) {
    return this.config[section] || {};
  }

  // 설정 값 수정
  set(section, key, value) {
    if (!this.config[section]) {
      this.config[section] = {};
    }
    this.config[section][key] = value;
    this._saveConfig();
  }

  // 설정 파일 저장
  _saveConfig() {
    try {
      fs.writeFileSync(this.configPath, ini.stringify(this.config), 'utf-8');
    } catch (error) {
      console.error('설정 파일 저장 실패:', error);
    }
  }

  // 전체 설정 리로드
  reload() {
    this.config = this._loadConfig();
  }
}

// 싱글톤 인스턴스 생성
const configManager = new ConfigManager('./config.ini');

module.exports = configManager;
