import os
import pwd
from functools import wraps

def run_as(username):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            user_info = pwd.getpwnam(username)
            original_uid = os.getuid()
            
            try:
                os.setuid(user_info.pw_uid)
                return func(*args, **kwargs)
            finally:
                os.setuid(original_uid)
        return wrapper
    return decorator

# 사용 예시
@run_as("nobody")
def sensitive_operation():
    import os
    return f"Performed by {os.getuid()}"

print(sensitive_operation())
