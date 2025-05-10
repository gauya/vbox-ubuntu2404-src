import subprocess

def capture_linux_command(command):
    try:
        process = subprocess.run(command, shell=True, capture_output=True, text=True, check=True)
        return process.stdout.strip()
    except subprocess.CalledProcessError as e:
        print(f"Error executing command '{command}': {e}")
        return None
    except FileNotFoundError:
        print(f"Error: Command '{command}' not found.")
        return None

# Capture the output of the 'df -h' command
df_output = capture_linux_command('df -h')
if df_output:
    print("Output of 'df -h':")
    print(df_output)
    # You can now store or further process the df_output string

# Capture the output of the 'top -n 1' command
top_output = capture_linux_command('top -n 1')
if top_output:
    print("\nOutput of 'top -n 1':")
    print(top_output)
    # You can now store or further process the top_output string

