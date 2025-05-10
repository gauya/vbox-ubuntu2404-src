import configparser
import os
import logging
from typing import Any, Dict, Optional, Callable
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler, FileModifiedEvent

class ConfigManager(FileSystemEventHandler):
    def __init__(self, config_path: str, auto_reload: bool = True):
        """
        ConfigManager 초기화
        
        :param config_path: 설정 파일 경로
        :param auto_reload: 파일 변경 시 자동 리로드 여부 (기본값 True)
        """
        self.config_path = os.path.abspath(config_path)
        self.config = configparser.ConfigParser()
        self.auto_reload = auto_reload
        self._callbacks = []
        self.observer = None
        self._setup_file_watcher()
        self._load_config()

    def _setup_file_watcher(self) -> None:
        """파일 변경 감지기 설정"""
        if self.auto_reload:
            self.observer = Observer()
            self.observer.schedule(self, path=os.path.dirname(self.config_path))
            self.observer.start()
            logging.info(f"Config file watcher started for {self.config_path}")

    def _load_config(self) -> None:
        """설정 파일 로드"""
        try:
            self.config.read(self.config_path)
            logging.info(f"Config loaded from {self.config_path}")
        except Exception as e:
            logging.error(f"설정 파일 로드 실패: {e}")
            self.config = configparser.ConfigParser()

    def on_modified(self, event: FileModifiedEvent) -> None:
        """파일 변경 이벤트 처리"""
        if os.path.abspath(event.src_path) == self.config_path:
            logging.info("Config file modified, reloading...")
            self._load_config()
            self._notify_callbacks()

    def register_callback(self, callback: Callable[[], None]) -> None:
        """설정 변경 시 호출될 콜백 등록"""
        self._callbacks.append(callback)

    def _notify_callbacks(self) -> None:
        """등록된 모든 콜백 호출"""
        for callback in self._callbacks:
            try:
                callback()
            except Exception as e:
                logging.error(f"Callback 실행 중 오류: {e}")

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

    def set(self, section: str, key: str, value: Any, save: bool = True) -> None:
        """설정 값 수정"""
        if not self.config.has_section(section):
            self.config.add_section(section)
        self.config.set(section, key, str(value))
        if save:
            self._save_config()

    def _save_config(self) -> None:
        """설정 파일 저장"""
        try:
            with open(self.config_path, 'w') as configfile:
                self.config.write(configfile)
            logging.info(f"Config saved to {self.config_path}")
        except Exception as e:
            logging.error(f"설정 파일 저장 실패: {e}")

    def reload(self) -> None:
        """설정 파일 강제 리로드"""
        self._load_config()
        self._notify_callbacks()

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

    def get_bool(self, section: str, key: str, default: bool = False) -> bool:
        """불리언형 설정 값 가져오기"""
        try:
            return self.config.getboolean(section, key)
        except (configparser.NoSectionError, configparser.NoOptionError):
            return default

    def close(self) -> None:
        """리소스 정리"""
        if self.observer:
            self.observer.stop()
            self.observer.join()
            logging.info("Config file watcher stopped")

    def __enter__(self):
        """with 문 지원"""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """with 문 종료 시 자동 정리"""
        self.close()

    def __del__(self):
        """객체 소멸 시 자동 정리"""
        self.close()
