use std::fs;
use std::process::Command;
use regex::Regex;
use std::collections::HashMap;

fn run_command(cmd: &str) -> Option<String> {
    let output = Command::new("sh")
        .arg("-c")
        .arg(cmd)
        .output();

    match output {
        Ok(out) => {
            if out.status.success() {
                Some(String::from_utf8_lossy(&out.stdout).trim().to_string())
            } else {
                eprintln!("Error running command '{}': {}", cmd, String::from_utf8_lossy(&out.stderr));
                None
            }
        }
        Err(e) => {
            eprintln!("Failed to execute command '{}': {}", cmd, e);
            None
        }
    }
}

fn get_repo_name() -> String {
    let remote_url = run_command("git remote get-url origin");
    if let Some(url) = remote_url {
        let re1 = Regex::new(r"[:/]([^/]+/[^/.]+)\.git$").unwrap();
        if let Some(caps) = re1.captures(&url) {
            let full_name = caps[1].to_string();
            if full_name.split('/').count() == 2 && full_name.split('/').next() == full_name.split('/').nth(1) {
                return full_name.split('/').nth(1).unwrap().to_string();
            }
            return full_name;
        }
        let re2 = Regex::new(r"[:/]([^/.]+)\.git$").unwrap();
        if let Some(caps) = re2.captures(&url) {
            return caps[1].to_string();
        }
    }
    let current_dir = std::env::current_dir().unwrap_or_default();
    current_dir.display().to_string()
}

fn get_languages() -> Vec<String> {
    let files_str = match run_command("git ls-files") {
        Some(s) => s,
        None => return vec!["Unknown".to_string()],
    };

    let files: Vec<&str> = files_str.lines().collect();
    let mut lang_counts: HashMap<String, usize> = HashMap::new();
    
    let ext_map = [
        ("py", "Python"),
        ("js", "JavaScript"),
        ("ts", "TypeScript"),
        ("md", "Markdown"),
        ("json", "JSON"),
        ("yml", "YAML"),
        ("yaml", "YAML"),
        ("html", "HTML"),
        ("css", "CSS"),
        ("c", "C"),
        ("cpp", "C++"),
        ("java", "Java"),
        ("go", "Go"),
        ("rs", "Rust"),
        ("sh", "Shell"),
        ("desktop", "Desktop"),
    ];

    for &file in &files {
        let path = std::path::Path::new(file);
        let ext = path.extension().and_then(|e| e.to_str()).unwrap_or("");
        let mut lang = "Other".to_string();
        
        for (ext_key, lang_name) in ext_map.iter() {
            if ext == *ext_key {
                lang = lang_name.to_string();
                break;
            }
        }
        *lang_counts.entry(lang).or_insert(0) += 1;
    }

    let mut sorted_langs: Vec<(String, usize)> = lang_counts.into_iter().collect();
    sorted_langs.sort_by(|a, b| {
        let a_is_other = a.0 == "Other";
        let b_is_other = b.0 == "Other";
        
        if a_is_other != b_is_other {
            if a_is_other { std::cmp::Ordering::Greater } else { std::cmp::Ordering::Less }
        } else {
            b.1.cmp(&a.1)
        }
    });

    let result: Vec<String> = sorted_langs.into_iter().take(3).map(|(lang, _)| lang).collect();
    println!("DEBUG LOG: Final Result: {:?}", result);
    result
}

fn get_latest_commit() -> String {
    match run_command("git log -1 --pretty=format:\"%s\"") {
        Some(c) => c,
        None => "No commits yet".to_string(),
    }
}

fn get_skill_level(num_files: usize, num_langs: usize) -> String {
    if num_files < 5 && num_langs < 2 {
        "Beginner 🌱".to_string()
    } else if num_files < 20 && num_langs < 5 {
        "Intermediate 🚀".to_string()
    } else {
        "Advanced ⚡".to_string()
    }
}

fn get_commit_count() -> String {
    match run_command("git rev-list --all --count") {
        Some(c) => c,
        None => "0".to_string(),
    }
}

fn update_readme() {
    let repo_name = get_repo_name();
    let files_str = match run_command("git ls-files") {
        Some(s) => s,
        None => "".to_string(),
    };
    
    let files: Vec<&str> = if files_str.is_empty() {
        vec![]
    } else {
        files_str.lines().collect()
    };

    let languages = get_languages();
    let lang_str = languages.join(", ");
    let commit_msg = get_latest_commit();
    let skill_level = get_skill_level(files.len(), languages.len());
    let commit_count = get_commit_count();

    let summary_content = format!(
        "\n- **Repository:** `{}`\n- **Primary Languages:** {}\n- **Total Commits:** {}\n- **Latest Activity:** {}\n- **Skill Level:** {}\n",
        repo_name, lang_str, commit_count, commit_msg, skill_level
    );

    let content = fs::read_to_string("README.md").unwrap_or_default();
    let re = Regex::new(r"(?s)<!-- REPO_SUMMARY_START -->.*?<!-- REPO_SUMMARY_END -->").unwrap();
    
    let final_content = re.replace(&content, &summary_content).to_string();

    fs::write("README.md", final_content).expect("Unable to write README.md");
    println!("README.md updated successfully.");
}

fn main() {
    update_readme();
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_skill_level_beginner() {
        assert_eq!(get_skill_level(4, 1), "Beginner 🌱");
    }

    #[test]
    fn test_get_skill_level_intermediate() {
        assert_eq!(get_skill_level(10, 3), "Intermediate 🚀");
    }

    #[test]
    fn test_get_skill_level_advanced() {
        assert_eq!(get_skill_level(25, 6), "Advanced ⚡");
    }
}
