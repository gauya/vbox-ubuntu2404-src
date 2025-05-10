# 개발/운영 환경별 설정 관리
env = os.getenv('APP_ENV', 'development')
config = ConfigManager(f'config.{env}.ini')

# 설정 검증
class ValidatedConfigManager(ConfigManager):
    REQUIRED_KEYS = {
        'database': ['host', 'user', 'password'],
        'server': ['port']
    }

    def _load_config(self):
        super()._load_config()
        self._validate_config()

    def _validate_config(self):
        for section, keys in self.REQUIRED_KEYS.items():
            if not self.config.has_section(section):
                raise ValueError(f"필수 섹션 '{section}'가 없습니다")
            for key in keys:
                if not self.config.has_option(section, key):
                    raise ValueError(f"섹션 '{section}'에 필수 키 '{key}'가 없습니다")


