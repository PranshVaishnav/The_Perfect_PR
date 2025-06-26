# C++ Code Guidelines Analyzer with AI Integration

A powerful Python tool that analyzes C++ code for coding guideline violations using English text-based rules **enhanced with AI-powered insights**. Combines traditional rule-based analysis with advanced AI code review capabilities using Groq AI. Perfect for PR reviews, CI/CD pipelines, and maintaining code quality standards.

## 🚀 Features

- **🤖 AI-Powered Analysis**: Advanced code review using Groq AI for deeper insights
- **📊 Dual Analysis**: Traditional guidelines + AI insights in one comprehensive report
- **English Text Guidelines**: Define coding rules in plain English, similar to Cursor rules
- **Multiple Analysis Modes**: Analyze individual files, GitHub PRs, git diffs, or staged files
- **Comprehensive C++ Support**: Covers naming conventions, best practices, formatting, and code structure
- **Multiple Output Formats**: Text reports, JSON, or GitHub PR comments with AI insights
- **Extensible Design**: Easy to add support for other programming languages
- **CI/CD Ready**: Perfect for automated code quality checks with AI enhancement

## 📦 Installation

### Prerequisites

- Python 3.7+
- Git (for analyzing git diffs and staged files)
- `requests` library for GitHub API access
- `groq` library for AI analysis (optional but recommended)

### Setup

1. Clone or download the analyzer files:
```bash
# Download the main files
curl -O https://raw.githubusercontent.com/your-repo/cpp_code_analyzer.py
curl -O https://raw.githubusercontent.com/your-repo/pr_analyzer.py

# Install dependencies
pip install requests groq
```

2. Set up your Groq API key for AI analysis:
```bash
# Get your free API key from https://console.groq.com
export GROQ_API_KEY="your-groq-api-key-here"
```

3. Make the scripts executable:
```bash
chmod +x cpp_code_analyzer.py pr_analyzer.py
```

## 🎯 Quick Start

### Analyze Individual Files

```bash
# Analyze a single C++ file (with AI insights)
python3 cpp_code_analyzer.py my_file.cpp

# Analyze multiple files with AI enhancement
python3 cpp_code_analyzer.py src/*.cpp include/*.h

# Use custom guidelines with AI analysis
python3 cpp_code_analyzer.py --guidelines custom_rules.json src/main.cpp

# Disable AI analysis (traditional guidelines only)
python3 cpp_code_analyzer.py --disable-ai my_file.cpp

# Use specific Groq API key
python3 cpp_code_analyzer.py --groq-api-key your-key-here my_file.cpp
```

### Analyze PR Changes

```bash
# Analyze files changed in current branch vs main (with AI)
python3 pr_analyzer.py --git-diff main

# Analyze staged files before commit (with AI insights)
python3 pr_analyzer.py --staged

# Analyze specific GitHub PR with AI enhancement
python3 pr_analyzer.py --pr owner/repo/123 --github-token YOUR_TOKEN

# Generate comprehensive PR comment with AI insights
python3 pr_analyzer.py --files *.cpp --format pr-comment

# Traditional analysis only (no AI)
python3 pr_analyzer.py --git-diff main --disable-ai

# Use specific Groq API key for PR analysis
python3 pr_analyzer.py --staged --groq-api-key your-key-here
```

## 🤖 AI Analysis Features

The analyzer now includes powerful AI-powered code review capabilities using Groq AI:

### What AI Analysis Provides:
- **Code Quality Assessment**: Overall quality score (1-10) for each file
- **Maintainability Score**: How easy the code is to maintain and extend
- **Performance Insights**: Potential bottlenecks and optimization opportunities
- **Security Analysis**: Identification of potential security vulnerabilities
- **Bug Detection**: Potential runtime errors and logic issues
- **Improvement Suggestions**: Specific, actionable recommendations

### Setting Up AI Analysis:

1. Get a free API key from [Groq Console](https://console.groq.com)
2. Set the environment variable:
   ```bash
   export GROQ_API_KEY="your-api-key-here"
   ```
3. Run the analyzer (AI analysis is enabled by default):
   ```bash
   python3 cpp_code_analyzer.py your_file.cpp
   ```

## 📋 Usage Examples

### 1. Comprehensive Analysis (Guidelines + AI)

```bash
$ python3 cpp_code_analyzer.py sample_code.cpp

📋 C++ Code Analysis Report
==================================================

## 📏 Guidelines Compliance
  🔴 Errors: 0
  🟡 Warnings: 2
  🔵 Info: 5

### 📁 sample_code.cpp
------
🟡 Line 4: Class names should use PascalCase (e.g., MyClass, HttpRequest)
   Code: class myClass {
   💡 Class name 'myClass' should use PascalCase

## 🤖 AI Code Analysis
==============================

### 📁 sample_code.cpp
------
**Overall Score:** 7/10
**Maintainability:** 8/10

**Code Quality Insights:**
The code demonstrates good structure but has some naming inconsistencies. The class design follows good encapsulation principles, but the mixed naming conventions reduce readability.

**💡 Improvement Suggestions:**
  1. Standardize naming conventions throughout the codebase
  2. Add comprehensive error handling for edge cases
  3. Consider using const-correctness for better safety

**🐛 Potential Issues:**
  1. Uninitialized member variables in constructor
  2. Missing null pointer checks in member functions
```

### 2. PR Analysis with AI Insights

```bash
# Set your tokens
export GITHUB_TOKEN="your_github_token_here"
export GROQ_API_KEY="your_groq_api_key_here"

# Analyze a specific PR with comprehensive AI analysis
python3 pr_analyzer.py --pr microsoft/vscode/12345 --github-token $GITHUB_TOKEN --format pr-comment

## 📋 Comprehensive Code Analysis Report

**Language:** CPP
**Files Analyzed:** 3

### 📏 Guidelines Compliance
- 🔴 **Errors:** 1
- 🟡 **Warnings:** 3
- 🔵 **Info:** 8

❗ **Please fix the errors before merging.**

#### Issues Found

**📁 `src/parser.h`**
🔴 **Line 1:** Header files should use include guards or #pragma once
💡 *Add #pragma once at the top of the header file*

### 🤖 AI Code Analysis

**Average Overall Score:** 7.3/10
**Average Maintainability:** 8.1/10

#### 📁 `src/parser.h`

**Quality Score:** 8/10 | **Maintainability:** 9/10

**💭 Key Insights:** The header file shows excellent design patterns with clear interfaces and good documentation. The class structure is well-organized and follows SOLID principles.

**💡 Top Suggestions:**
- Consider using std::optional for nullable return values
- Add noexcept specifications to non-throwing functions
- Consider implementing move semantics for better performance

**🔒 Security Concerns:**
- Buffer boundary checks should be added for array operations
```

### 3. Git Integration

```bash
# Check what you're about to commit
git add .
python3 pr_analyzer.py --staged

# Compare against different branch
python3 pr_analyzer.py --git-diff develop

# JSON output for CI/CD
python3 pr_analyzer.py --git-diff main --format json --output violations.json
```

### 4. Quick Demo

Run the included demo to test AI integration:

```bash
# Set your API key
export GROQ_API_KEY="your-groq-api-key-here"

# Run the demo (analyzes sample files)
python3 demo_ai_analysis.py

🚀 C++ Code Analyzer with AI Integration Demo
==================================================
✅ Found Groq API key: gsk_...XYZ
📁 Analyzing 2 file(s): sample_code.cpp, sample_header.h

🔍 Analyzing sample_code.cpp...
🤖 Running AI analysis for sample_code.cpp...
   Found 5 guideline violations

📊 Analysis Summary:
   Total violations: 8
   AI analyses: 2

💾 Report saved to 'analysis_report.txt'
💾 JSON report saved to 'analysis_report.json'
```

## ⚙️ Configuration

### Custom Guidelines

Create a custom guidelines file (`custom_cpp_rules.json`):

```json
{
  "naming_conventions": {
    "class_names": {
      "rule": "Class names must use PascalCase starting with a capital letter",
      "pattern": "^[A-Z][a-zA-Z0-9]*$",
      "severity": "warning",
      "examples": {
        "good": ["MyClass", "HttpRequest", "FileReader"],
        "bad": ["myClass", "http_request", "file_reader"]
      }
    },
    "function_names": {
      "rule": "Function names should use camelCase consistently",
      "pattern": "^[a-z][a-zA-Z0-9]*$",
      "severity": "warning",
      "examples": {
        "good": ["calculateSum", "processData", "getValue"],
        "bad": ["CalculateSum", "process_data", "get_Value"]
      }
    }
  },
  "best_practices": {
    "memory_management": {
      "rule": "Prefer smart pointers over raw pointers",
      "keywords": ["new", "delete", "malloc", "free"],
      "severity": "info",
      "suggestion": "Use std::unique_ptr, std::shared_ptr, or RAII patterns"
    }
  }
}
```

Use your custom rules:
```bash
python3 cpp_code_analyzer.py --guidelines custom_cpp_rules.json src/
```

### Default Guidelines

The analyzer includes comprehensive default guidelines covering:

#### Naming Conventions
- **Class Names**: PascalCase (e.g., `MyClass`, `HttpRequest`)
- **Function Names**: camelCase or snake_case consistently
- **Variables**: snake_case for locals, camelCase or m_ prefix for members
- **Constants**: UPPER_SNAKE_CASE

#### Code Structure
- **Line Length**: Maximum 120 characters
- **Include Guards**: Required for header files
- **Function Length**: Maximum 50 lines

#### Best Practices
- **Memory Management**: Prefer smart pointers over raw pointers
- **Null Checks**: Validate pointers before dereferencing
- **Namespace Usage**: Avoid `using namespace std` in headers
- **Const Correctness**: Use const where appropriate

#### Formatting
- **Trailing Whitespace**: Remove trailing spaces
- **Consistent Indentation**: 2 or 4 spaces (no tabs)
- **Brace Style**: Consistent placement

## 🔧 Command Line Options

### cpp_code_analyzer.py

```bash
usage: cpp_code_analyzer.py [-h] [--guidelines GUIDELINES] [--format {text,json}] [--output OUTPUT] files [files ...]

Arguments:
  files                 C++ files to analyze

Options:
  --guidelines         Custom guidelines JSON file
  --format {text,json} Output format (default: text)
  --output OUTPUT      Output file (default: stdout)
```

### pr_analyzer.py

```bash
usage: pr_analyzer.py [-h] [--pr PR] [--files FILES [FILES ...]] [--git-diff [GIT_DIFF]] [--staged] 
                     [--language LANGUAGE] [--format {text,json,pr-comment}] [--output OUTPUT] 
                     [--github-token GITHUB_TOKEN]

Options:
  --pr PR                    GitHub PR in format 'owner/repo/pr_number'
  --files FILES [FILES ...]  Specific files to analyze
  --git-diff [GIT_DIFF]     Compare against branch (default: main)
  --staged                   Analyze staged files
  --language LANGUAGE        Programming language (default: cpp)
  --format {text,json,pr-comment}  Output format (default: text)
  --output OUTPUT           Output file (default: stdout)
  --github-token GITHUB_TOKEN  GitHub token for API access
```

## 🔄 CI/CD Integration

### GitHub Actions

```yaml
name: Code Quality Check

on:
  pull_request:
    branches: [ main ]

jobs:
  code-analysis:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
      with:
        fetch-depth: 0
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install dependencies
      run: pip install requests
    
    - name: Run C++ Code Analysis
      run: |
        python3 pr_analyzer.py --git-diff origin/main --format json --output analysis.json
        
    - name: Comment PR
      if: github.event_name == 'pull_request'
      run: |
        python3 pr_analyzer.py --git-diff origin/main --format pr-comment --output comment.md
        # Use GitHub CLI or API to post comment.md to the PR
```

### Pre-commit Hook

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash
# Run code analysis on staged files
python3 pr_analyzer.py --staged --format text

# Exit with error code if there are critical violations
python3 pr_analyzer.py --staged --format json | jq -e '.summary.errors == 0' > /dev/null
if [ $? -ne 0 ]; then
    echo "❌ Critical errors found. Please fix before committing."
    exit 1
fi
```

## 🎨 Output Formats

### Text Format (Default)
Human-readable report with emojis and color coding.

### JSON Format
Structured data perfect for CI/CD integration:

```json
{
  "summary": {
    "total_violations": 5,
    "errors": 1,
    "warnings": 2,
    "info": 2
  },
  "violations": [
    {
      "rule_name": "include_guards",
      "description": "Header files should use include guards or #pragma once",
      "file_path": "src/parser.h",
      "line_number": 1,
      "severity": "error",
      "suggestion": "Add #pragma once at the top of the header file"
    }
  ]
}
```

### PR Comment Format
GitHub-flavored markdown perfect for automated PR comments.

## 🛠️ Extending the Analyzer

### Adding New Languages

1. Create a new analyzer class (e.g., `PythonGuidelinesAnalyzer`)
2. Implement the same interface as `CppGuidelinesAnalyzer`
3. Add language detection in `pr_analyzer.py`

### Adding New Rules

Add rules to your custom guidelines JSON:

```json
{
  "custom_category": {
    "rule_name": {
      "rule": "English description of the rule",
      "pattern": "regex_pattern",
      "severity": "error|warning|info",
      "suggestion": "How to fix this issue"
    }
  }
}
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## 📝 License

MIT License - feel free to use in your projects!

## 🔗 Related Tools

This analyzer was inspired by tools like:
- [Qodo AI](https://qodo.ai) - AI-powered code integrity platform
- Cursor IDE rules
- Traditional linters like cpplint, clang-tidy

## 📞 Support

- 🐛 **Issues**: Report bugs or request features
- 📖 **Documentation**: Check this README for comprehensive usage
- 💬 **Discussions**: Ask questions or share improvements

---

**Happy coding! 🚀** 