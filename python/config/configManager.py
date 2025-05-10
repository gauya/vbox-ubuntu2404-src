import configparser
import os
from typing import Any, Dict, Optional

class ConfigManager:
    def __init__(self, config_path: str):
        self.config_path = os.path.abspath(config_path)
        self.config = configparser.ConfigParser()
        self._load_config()

    def _load_config(self) -> None:
        """설정 파일 로드"""
        try:
            self.config.read(self.config_path)
        except Exception as e:
            print(f"설정 파일 로드 실패: {e}")
            self.config = configparser.ConfigParser()

    def get(self, section: str, key: str, default: Any = None) -> Any:
        """설정 값 가져오기"""
        try:
            return self.config.get(section, key)
        except (configparser.NoSectionError, configparser.NoOptionError):
            return default

    def get_section(self, section: str) -> Dict[str, str]:
        """전체 섹션 가져오기"""
        try:
            return dict(self.config.items(section))
        except configparser.NoSectionError:
            return {}

    def set(self, section: str, key: str, value: Any) -> None:
        """설정 값 수정"""
        if not self.config.has_section(section):
            self.config.add_section(section)
        self.config.set(section, key, str(value))
        self._save_config()

    def _save_config(self) -> None:
        """설정 파일 저장"""
        try:
            with open(self.config_path, 'w') as configfile:
                self.config.write(configfile)
        except Exception as e:
            print(f"설정 파일 저장 실패: {e}")

    def reload(self) -> None:
        """설정 파일 다시 로드"""
        self._load_config()

    def get_int(self, section: str, key: str, default: int = 0) -> int:
        """정수형 설정 값 가져오기"""
        try:
            return self.config.getint(section, key)
        except (configparser.NoSectionError, configparser.NoOptionError):
            return default

    def get_float(self, section: str, key: str, default: float = 0.0) -> float:
        """부동소수점형 설정 값 가져오기"""
        try:
            return self.config.getfloat(section, key)
        except (configparser.NoSectionError, configparser.NoOptionError):
            return default

    def get_boolean(self, section: str, key: str, default: bool = False) -> bool:
        """불리언형 설정 값 가져오기"""
        try:
            return self.config.getboolean(section, key)
        except (configparser.NoSectionError, configparser.NoOptionError):
            return default
