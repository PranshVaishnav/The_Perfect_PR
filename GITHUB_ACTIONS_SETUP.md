# GitHub Actions Setup Guide for C++ Code Analyzer with AI Integration

This guide will help you set up automated C++ code analysis with AI-powered insights using GitHub Actions for your repository.

## 📁 Repository Structure

Ensure your repository has this structure:
```
your-repo/
├── .github/
│   └── workflows/
│       └── cpp-guidelines-check.yml  # GitHub Actions workflow
├── cpp_code_analyzer.py              # Your C++ analyzer
├── cpp_guidelines.json               # Optional: Custom guidelines
└── your-cpp-files/
    ├── *.cpp
    ├── *.h
    └── ...
```

## 🚀 Quick Setup

### Step 1: Copy the Workflow File
The workflow file is already created at `.github/workflows/cpp-guidelines-check.yml`. This file will:
- ✅ Trigger on PRs to `main` and `develop` branches
- ✅ Only run when C++ files are changed
- ✅ Post detailed comments on PRs with AI insights
- ✅ Upload analysis artifacts
- ✅ Fail the build if errors are found
- 🤖 Run AI analysis if Groq API key is available

### Step 2: Set Up AI Analysis (Optional but Recommended)

1. **Get a Groq API Key**:
   - Visit [Groq Console](https://console.groq.com)
   - Sign up for a free account
   - Create an API key

2. **Add API Key to Repository Secrets**:
   - Go to your repository settings
   - Navigate to "Secrets and variables" → "Actions"
   - Click "New repository secret"
   - Name: `GROQ_API_KEY`
   - Value: Your Groq API key

3. **Note**: If no API key is provided, the workflow will run traditional analysis only

### Step 3: Repository Settings

1. **Enable GitHub Actions** (if not already enabled):
   - Go to your repository settings
   - Navigate to "Actions" → "General"
   - Ensure "Allow all actions and reusable workflows" is selected

2. **Set Branch Protection** (optional but recommended):
   - Go to "Settings" → "Branches"
   - Add a rule for your main branch
   - Check "Require status checks to pass before merging"
   - Select "C++ Code Guidelines Check" from the list

### Step 3: Test the Setup

1. **Create a test PR** with some C++ code changes
2. **Watch the Actions tab** to see the workflow run
3. **Check the PR comments** for automated analysis results

## 🔧 Configuration Options

### Customize Analyzed Branches
Edit the workflow file to change which branches trigger analysis:
```yaml
on:
  pull_request:
    branches: [ main, develop, feature/* ]  # Add your branches here
```

### Customize File Extensions
Add or remove C++ file extensions in the `paths` section:
```yaml
paths:
  - '**/*.cpp'
  - '**/*.cc'
  - '**/*.cxx'
  - '**/*.c'
  - '**/*.hpp'
  - '**/*.h'
  - '**/*.hxx'
  - '**/*.C'     # Add uppercase extensions if needed
  - '**/*.H'
```

### Custom Guidelines
If you have a custom guidelines JSON file:
1. Create `cpp_guidelines.json` in your repository root
2. Modify the workflow to use it:
```yaml
- name: Run C++ Guidelines Analyzer
  run: |
    python3 cpp_code_analyzer.py --guidelines cpp_guidelines.json --format json --output analysis_result.json ${{ steps.changed-files.outputs.files }}
```

### Error vs Warning Behavior
The workflow currently:
- ❌ **FAILS** the build if errors are found (severity: "error")
- ⚠️ **WARNS** but continues if only warnings are found (severity: "warning")
- ℹ️ **PASSES** with info messages (severity: "info")

To change this behavior, modify the final step in the workflow.

## 📊 What the Workflow Does

### 1. **Automatic Triggering**
- Runs on every PR to specified branches
- Only when C++ files are modified
- Can be manually triggered from the Actions tab

### 2. **File Analysis**
- Detects changed C++ files using `git diff`
- Runs your `cpp_code_analyzer.py` on only the changed files
- Generates both JSON and text reports

### 3. **PR Comments**
The bot will post comprehensive comments like this:

```markdown
## 📋 Comprehensive Code Analysis Report

*Last updated: 2024-01-15T10:30:00.000Z*

🤖 **AI Analysis**: Enabled

### 📏 Guidelines Compliance
- 🔴 **Errors**: 1
- 🟡 **Warnings**: 5  
- 🔵 **Info**: 2
- **Total**: 8 violations

### 🤖 AI Code Analysis
- **Average Quality Score**: 7.3/10
- **Average Maintainability**: 8.1/10
- **Files Analyzed**: 3

❌ **Status**: BLOCKING - This PR cannot be merged due to errors

<details>
<summary>📝 Detailed Report (click to expand)</summary>

[Comprehensive analysis report with both guideline violations and AI insights...]

</details>

### 💡 Quick Fixes
Some common issues and their fixes:
- **Naming**: Use PascalCase for classes/functions, camelCase for variables, m_ prefix for members
- **Headers**: Add include guards (#pragma once) and avoid 'using namespace std'
- **Modern C++**: Use smart pointers instead of raw pointers, prefer nullptr over NULL
- **Comments**: Add Doxygen-style comments for public functions in headers

---
*This analysis combines traditional C++ guidelines checking with AI-powered insights for comprehensive code review.*
```

### 4. **Artifacts**
- Analysis results are uploaded as artifacts
- Available for download for 30 days
- Includes both JSON and text formats

### 5. **Status Checks**
- ✅ **Passes** if no errors found
- ❌ **Fails** if errors found (blocks merge if branch protection is enabled)
- ⚠️ **Warns** if only warnings found

## 🛠️ Troubleshooting

### Common Issues

1. **"Permission denied" errors**
   - Ensure the workflow has `pull-requests: write` permission
   - Check your repository's Actions permissions

2. **"Python not found" errors**
   - The workflow uses `python3` - make sure your analyzer script uses the same

3. **"No C++ files changed" but expecting analysis**
   - Check that your file extensions match the `paths` configuration
   - Verify files were actually modified (not just moved/renamed)

4. **Analysis not posting comments**
   - Check the Actions logs for errors
   - Ensure the bot has permission to comment on PRs
   - Verify the PR is from a branch in the same repository (not a fork)

### Debug Mode
To enable more verbose logging, add this to any step:
```yaml
env:
  ACTIONS_STEP_DEBUG: true
```

### Manual Testing
You can test the analyzer locally before pushing:
```bash
# Test on specific files
python3 cpp_code_analyzer.py --format text my_file.cpp my_header.h

# Test with JSON output  
python3 cpp_code_analyzer.py --format json --output result.json my_file.cpp
```

## 🔄 Advanced Configurations

### Multiple Environments
Run analysis on different OS/Python versions:
```yaml
strategy:
  matrix:
    os: [ubuntu-latest, windows-latest, macos-latest]
    python-version: [3.8, 3.9, '3.10']
```

### Custom Notification Channels
Send results to Slack, Teams, or email by adding notification steps.

### Integration with Other Tools
Chain with other code quality tools like:
- Static analysis (cppcheck, clang-static-analyzer)
- Code formatting (clang-format)
- Documentation generation (doxygen)

## 📋 Checklist

- [ ] Workflow file created in `.github/workflows/cpp-guidelines-check.yml`
- [ ] Repository Actions are enabled
- [ ] Branch protection rules configured (optional)
- [ ] Test PR created and workflow runs successfully
- [ ] PR comments appear as expected
- [ ] Custom guidelines configured (if needed)
- [ ] Team trained on reading analysis results

## 🎉 You're All Set!

Your C++ code analyzer is now integrated with GitHub Actions! Every PR will automatically be analyzed for coding guideline violations, helping maintain consistent code quality across your project.

The workflow will:
- 🔍 **Analyze** changed C++ files automatically
- 💬 **Comment** on PRs with detailed results  
- 🚫 **Block** merges if errors are found (with branch protection)
- 📊 **Track** code quality trends over time 