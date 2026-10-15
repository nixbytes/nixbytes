import os
import subprocess
import re

def run_command(command):
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True, check=True)
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        print(f"Error running command '{command}': {e.stderr}")
        return None

def get_repo_name():
    # Try to get repo name from git remote
    remote_url = run_command("git remote get-url origin")
    if remote_url:
        # URL can be https://github.com/user/repo.git or git@github.com:user/repo.git
        match = re.search(r'[:/]([^/]+/[^/.]+)\.git$', remote_url)
        if match:
            return match.group(1)
        match = re.search(r'[:/]([^/.]+)\.git$', remote_url)
        if match:
            return match.group(1)
    return os.path.basename(os.getcwd())

def get_languages():
    files = run_command("git ls-files").split('\n')
    if not files or files == ['']:
        return ["Unknown"]
    
    ext_map = {
        '.py': 'Python',
        '.js': 'JavaScript',
        '.ts': 'TypeScript',
        '.md': 'Markdown',
        '.json': 'JSON',
        '.yml': 'YAML',
        '.yaml': 'YAML',
        '.html': 'HTML',
        '.css': 'CSS',
        '.c': 'C',
        '.cpp': 'C++',
        '.java': 'Java',
        '.go': 'Go',
        '.rs': 'Rust',
        '.sh': 'Shell',
        '.desktop': 'Desktop',
    }
    
    lang_counts = {}
    for f in files:
        _, ext = os.path.splitext(f)
        lang = ext_map.get(ext, 'Other')
        lang_counts[lang] = lang_counts.get(lang, 0) + 1
    
    # Sort by count and return top languages
    sorted_langs = sorted(lang_counts.items(), key=lambda x: x[1], reverse=True)
    return [lang for lang, count in sorted_langs]

def get_latest_commit():
    commit = run_command('git log -1 --pretty=format:"%s"')
    return commit if commit else "No commits yet"

def get_skill_level(files, languages):
    num_files = len(files)
    num_langs = len(languages)
    
    # Very simple heuristic for "skill level"
    if num_files < 5 and num_langs < 2:
        return "Beginner 🌱"
    elif num_files < 20 and num_langs < 5:
        return "Intermediate 🚀"
    else:
        return "Advanced ⚡"

def update_readme():
    repo_name = get_repo_name()
    files = run_command("git ls-files").split('\n')
    if files == ['']: files = []
    languages = get_languages()
    lang_str = ", ".join(languages)
    commit_msg = get_latest_commit()
    skill_level = get_skill_level(files, languages)

    summary_content = f"""## 📊 Repository Summary

- **Repo:** `{repo_name}`
- **Languages:** {lang_str}
- **Latest Commit:** {commit_msg}
- **Skill Level:** {skill_level}
"""

    with open("README.md", "r") as f:
        content = f.read()

    pattern = re.compile(r"<!-- REPO_SUMMARY_START -->.*?<!-- REPO_SUMMARY_END -->", re.DOTALL)
    new_content = pattern.sub(f"<!-- REPO_SUMMARY_START -->\n{summary_content}\n<!-- REPO_SUMMARY_END -->", content)

    with open("README.md", "w") as f:
        f.write(new_content)
    print("README.md updated successfully.")

if __name__ == "__main__":
    update_readme()
