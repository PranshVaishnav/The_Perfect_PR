#!/usr/bin/env python3
"""
C++ Code Guidelines Analyzer with AI Integration
Analyzes C++ files for coding guideline violations and provides AI-powered insights.
"""

import os
import re
import json
from typing import List, Dict, Any, Tuple, Optional
from dataclasses import dataclass
from pathlib import Path
import argparse
try:
    from groq import Groq
except ImportError:
    Groq = None


@dataclass
class Violation:
    """Represents a coding guideline violation."""
    rule_name: str
    description: str
    file_path: str
    line_number: int
    line_content: str
    severity: str  # 'error', 'warning', 'info'
    suggestion: str = ""


@dataclass
class AIAnalysis:
    """Represents AI-powered code analysis results."""
    file_path: str
    overall_score: int  # 1-10 scale
    code_quality_insights: str
    improvement_suggestions: List[str]
    potential_bugs: List[str]
    security_concerns: List[str]
    performance_insights: str
    maintainability_score: int  # 1-10 scale


class GroqAIAnalyzer:
    """AI-powered code analyzer using Groq."""
    
    def __init__(self, api_key: Optional[str] = None):
        if not Groq:
            raise ImportError("groq package not installed. Install with: pip install groq")
        
        self.api_key = api_key or os.getenv('GROQ_API_KEY')
        if not self.api_key:
            raise ValueError("Groq API key not provided. Set GROQ_API_KEY environment variable or pass api_key parameter.")
        
        self.client = Groq(api_key=self.api_key)
        
    def analyze_code(self, file_path: str, code_content: str) -> AIAnalysis:
        """Analyze C++ code using Groq AI."""
        try:
            prompt = self._create_analysis_prompt(file_path, code_content)
            
            response = self.client.chat.completions.create(
                messages=[
                    {
                        "role": "system",
                        "content": "You are an expert C++ code reviewer and software architect. Analyze the provided C++ code and provide detailed insights about code quality, potential issues, and improvements."
                    },
                    {
                        "role": "user",
                        "content": prompt
                    }
                ],
                model="llama3-8b-8192",  # or "mixtral-8x7b-32768"
                temperature=0.1,
                max_tokens=2048
            )
            
            content = response.choices[0].message.content or ""
            return self._parse_ai_response(file_path, content)
            
        except Exception as e:
            print(f"Error analyzing code with AI: {e}")
            return self._create_fallback_analysis(file_path)
    
    def _create_analysis_prompt(self, file_path: str, code_content: str) -> str:
        """Create a detailed prompt for AI analysis."""
        return f"""
Please analyze the following C++ code from file '{file_path}':

```cpp
{code_content}
```

Provide a comprehensive analysis in the following JSON format:
{{
    "overall_score": <1-10 integer>,
    "code_quality_insights": "<detailed assessment of code quality>",
    "improvement_suggestions": ["<suggestion 1>", "<suggestion 2>", ...],
    "potential_bugs": ["<potential bug 1>", "<potential bug 2>", ...],
    "security_concerns": ["<security issue 1>", "<security issue 2>", ...],
    "performance_insights": "<performance analysis and suggestions>",
    "maintainability_score": <1-10 integer>
}}

Focus on:
1. Code structure and organization
2. Memory management and resource handling
3. Potential runtime errors or undefined behavior
4. Security vulnerabilities (buffer overflows, injection points, etc.)
5. Performance bottlenecks and optimization opportunities
6. Code maintainability and readability
7. Modern C++ best practices compliance
8. Thread safety considerations (if applicable)

Provide specific, actionable suggestions. If the code is excellent, mention that too.
"""
    
    def _parse_ai_response(self, file_path: str, response_content: str) -> AIAnalysis:
        """Parse AI response and create AIAnalysis object."""
        try:
            # Try to extract JSON from the response
            json_start = response_content.find('{')
            json_end = response_content.rfind('}') + 1
            
            if json_start != -1 and json_end > json_start:
                json_content = response_content[json_start:json_end]
                data = json.loads(json_content)
                
                return AIAnalysis(
                    file_path=file_path,
                    overall_score=int(data.get('overall_score', 5)),
                    code_quality_insights=data.get('code_quality_insights', ''),
                    improvement_suggestions=data.get('improvement_suggestions', []),
                    potential_bugs=data.get('potential_bugs', []),
                    security_concerns=data.get('security_concerns', []),
                    performance_insights=data.get('performance_insights', ''),
                    maintainability_score=int(data.get('maintainability_score', 5))
                )
            else:
                return self._create_fallback_analysis_with_content(file_path, response_content)
                
        except (json.JSONDecodeError, KeyError, ValueError) as e:
            print(f"Error parsing AI response: {e}")
            return self._create_fallback_analysis_with_content(file_path, response_content)
    
    def _create_fallback_analysis(self, file_path: str) -> AIAnalysis:
        """Create a fallback analysis when AI analysis fails."""
        return AIAnalysis(
            file_path=file_path,
            overall_score=5,
            code_quality_insights="AI analysis unavailable - please check your API key and connection.",
            improvement_suggestions=["AI analysis could not be performed"],
            potential_bugs=[],
            security_concerns=[],
            performance_insights="AI analysis unavailable",
            maintainability_score=5
        )
    
    def _create_fallback_analysis_with_content(self, file_path: str, content: str) -> AIAnalysis:
        """Create analysis from non-JSON AI response."""
        return AIAnalysis(
            file_path=file_path,
            overall_score=6,
            code_quality_insights=content[:500] + "..." if len(content) > 500 else content,
            improvement_suggestions=["See detailed insights for suggestions"],
            potential_bugs=[],
            security_concerns=[],
            performance_insights="Included in general insights",
            maintainability_score=6
        )


class CppGuidelinesAnalyzer:
    """Analyzes C++ code against comprehensive C++ guidelines with AI integration."""
    
    def __init__(self, guidelines_file: Optional[str] = None, groq_api_key: Optional[str] = None, enable_ai: bool = True):
        self.guidelines = self.load_guidelines(guidelines_file)
        self.violations: List[Violation] = []
        self.ai_analyses: List[AIAnalysis] = []
        self.enable_ai = enable_ai and Groq is not None
        
        if self.enable_ai:
            try:
                self.ai_analyzer = GroqAIAnalyzer(groq_api_key)
            except (ImportError, ValueError) as e:
                print(f"AI analysis disabled: {e}")
                self.enable_ai = False
                self.ai_analyzer = None
        else:
            self.ai_analyzer = None
    
    def load_guidelines(self, guidelines_file: Optional[str] = None) -> Dict[str, Any]:
        """Load guidelines from file or use default guidelines."""
        if guidelines_file and os.path.exists(guidelines_file):
            with open(guidelines_file, 'r') as f:
                return json.load(f)
        else:
            return self.get_default_cpp_guidelines()
    
    def get_default_cpp_guidelines(self) -> Dict[str, Any]:
        """Comprehensive C++ coding guidelines."""
        return {
            "naming_conventions": {
                "class_names": {
                    "rule": "Class names should use PascalCase (e.g., MyClass, UrlTable)",
                    "pattern": r"^[A-Z][a-zA-Z0-9]*$",
                    "severity": "warning",
                    "applies_to": ["class", "struct"]
                },
                "function_names": {
                    "rule": "Function names should use PascalCase (e.g., MyExcitingFunction, AddTableEntry)",
                    "pattern": r"^[A-Z][a-zA-Z0-9]*$",
                    "severity": "warning",
                    "applies_to": ["function", "method"]
                },
                "variable_names": {
                    "rule": "Variable names should use camelCase starting with lowercase (e.g., tableName, priceCountReader)",
                    "pattern": r"^[a-z][a-zA-Z0-9]*$",
                    "severity": "warning",
                    "applies_to": ["variable"]
                },
                "member_variables": {
                    "rule": "Class member variables should be prefixed with 'm_' and use camelCase (e.g., m_tableName, m_someVar)",
                    "pattern": r"^m_[a-z][a-zA-Z0-9]*$",
                    "severity": "warning",
                    "applies_to": ["member_variable"]
                },
                "constant_names": {
                    "rule": "Constants should use 'k' prefix followed by PascalCase (e.g., kDaysInAWeek, kCreateDrive)",
                    "pattern": r"^k[A-Z][a-zA-Z0-9]*$",
                    "severity": "warning",
                    "applies_to": ["constant"]
                },
                "global_variables": {
                    "rule": "Global variables should be prefixed with 'g_' or similar marker",
                    "pattern": r"^g_[a-zA-Z][a-zA-Z0-9]*$",
                    "severity": "info",
                    "applies_to": ["global_variable"]
                },
                "namespace_names": {
                    "rule": "Namespace names should start with capital and follow camelCase (e.g., AdDelivery)",
                    "pattern": r"^[A-Z][a-zA-Z0-9]*$",
                    "severity": "warning",
                    "applies_to": ["namespace"]
                }
            },
            "formatting": {
                "line_length": {
                    "rule": "Lines should not exceed 120 characters",
                    "max_length": 120,
                    "severity": "info"
                },
                "indentation": {
                    "rule": "Use 2 spaces for indentation (no tabs)",
                    "spaces": 2,
                    "severity": "warning"
                },
                "brace_style": {
                    "rule": "Opening braces should be on the same line as declaration",
                    "severity": "info"
                },
                "trailing_whitespace": {
                    "rule": "Remove trailing whitespace from lines",
                    "pattern": r"\s+$",
                    "severity": "info"
                },
                "space_after_keywords": {
                    "rule": "Space required between keywords (if, for, while) and parentheses",
                    "pattern": r"\b(if|for|while|switch|catch)\(",
                    "severity": "warning"
                },
                "no_space_inside_parens": {
                    "rule": "No spaces inside parentheses for conditions",
                    "pattern": r"\(\s+|\s+\)",
                    "severity": "info"
                }
            },
            "code_structure": {
                "include_guards": {
                    "rule": "Header files should use include guards (#ifndef PROJECT_PATH_FILE_H_)",
                    "severity": "error",
                    "applies_to": ["header"]
                },
                "function_length": {
                    "rule": "Functions should not exceed 50 lines to maintain readability",
                    "max_lines": 50,
                    "severity": "warning"
                },
                "function_parameters": {
                    "rule": "Function parameters should be limited to 3-4 maximum",
                    "max_params": 4,
                    "severity": "info"
                },
                "class_access_order": {
                    "rule": "Class sections should be in order: public, protected, private",
                    "severity": "info"
                }
            },
            "best_practices": {
                "smart_pointers": {
                    "rule": "Prefer smart pointers over raw pointers (std::shared_ptr, std::unique_ptr, std::weak_ptr)",
                    "keywords": ["new", "delete"],
                    "severity": "info",
                    "suggestion": "Consider using std::unique_ptr, std::shared_ptr, or RAII patterns"
                },
                "nullptr_usage": {
                    "rule": "Use nullptr instead of NULL or 0 for pointers",
                    "pattern": r"\b(NULL|0)\b(?=\s*[;,)])",
                    "severity": "warning",
                    "suggestion": "Use nullptr for pointer initialization"
                },
                "const_correctness": {
                    "rule": "Use const keyword where appropriate for better code safety",
                    "severity": "info"
                },
                "namespace_std_in_headers": {
                    "rule": "Avoid 'using namespace std' in header files",
                    "pattern": r"using\s+namespace\s+std\s*;",
                    "severity": "warning",
                    "applies_to": ["header"]
                },
                "explicit_constructors": {
                    "rule": "Single-parameter constructors should be explicit",
                    "pattern": r"^\s*[A-Z][a-zA-Z0-9]*\s*\([^,)]+\)\s*[:{]",
                    "severity": "info"
                },
                "member_initialization": {
                    "rule": "Initialize member variables in constructor initializer lists",
                    "severity": "info"
                }
            },
            "modern_cpp": {
                "auto_usage": {
                    "rule": "Use auto for local variables when type is obvious, avoid for class members",
                    "severity": "info"
                },
                "lambda_captures": {
                    "rule": "Write all lambda captures explicitly (no default captures)",
                    "pattern": r"\[=\]|\[\&\]",
                    "severity": "warning",
                    "suggestion": "Use explicit captures instead of [=] or [&]"
                },
                "range_based_for": {
                    "rule": "Prefer range-based for loops when appropriate",
                    "severity": "info"
                }
            },
            "comments": {
                "function_comments": {
                    "rule": "All functions must have Doxygen-style comments in header files",
                    "severity": "warning",
                    "applies_to": ["header"]
                },
                "class_comments": {
                    "rule": "All classes should have descriptive comments",
                    "severity": "info",
                    "applies_to": ["header"]
                }
            }
        }
    
    def analyze_file(self, file_path: str) -> List[Violation]:
        """Analyze a single C++ file for guideline violations and AI insights."""
        if not file_path.endswith(('.cpp', '.cc', '.cxx', '.c', '.hpp', '.h', '.hxx')):
            return []
        
        violations = []
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
                code_content = ''.join(lines)
            
            is_header = file_path.endswith(('.h', '.hpp', '.hxx'))
            
            # Perform traditional guideline checks
            violations.extend(self._check_line_length(file_path, lines))
            violations.extend(self._check_formatting(file_path, lines))
            violations.extend(self._check_naming_conventions(file_path, lines))
            violations.extend(self._check_best_practices(file_path, lines, is_header))
            violations.extend(self._check_code_structure(file_path, lines, is_header))
            violations.extend(self._check_modern_cpp(file_path, lines))
            violations.extend(self._check_comments(file_path, lines, is_header))
            
            # Perform AI analysis if enabled
            if self.enable_ai and self.ai_analyzer:
                print(f"🤖 Running AI analysis for {file_path}...")
                ai_analysis = self.ai_analyzer.analyze_code(file_path, code_content)
                self.ai_analyses.append(ai_analysis)
            
        except Exception as e:
            print(f"Error analyzing file {file_path}: {e}")
        
        return violations
    
    def _check_line_length(self, file_path: str, lines: List[str]) -> List[Violation]:
        """Check for lines exceeding maximum length."""
        violations = []
        max_length = self.guidelines["formatting"]["line_length"]["max_length"]
        
        for i, line in enumerate(lines, 1):
            line_content = line.rstrip()
            if len(line_content) > max_length:
                # Allow exceptions for certain cases
                if (line.strip().startswith('//') or  # Long comments
                    line.strip().startswith('#include') or  # Include statements  
                    line.strip().startswith('#ifndef') or  # Header guards
                    'http' in line.lower()):  # URLs
                    continue
                    
                violations.append(Violation(
                    rule_name="line_length",
                    description=self.guidelines["formatting"]["line_length"]["rule"],
                    file_path=file_path,
                    line_number=i,
                    line_content=line_content,
                    severity=self.guidelines["formatting"]["line_length"]["severity"],
                    suggestion=f"Consider breaking this line into multiple lines (current: {len(line_content)} chars)"
                ))
        
        return violations
    
    def _check_formatting(self, file_path: str, lines: List[str]) -> List[Violation]:
        """Check formatting issues."""
        violations = []
        
        for i, line in enumerate(lines, 1):
            # Check trailing whitespace (but not empty lines)
            # Remove newline first, then check for trailing spaces/tabs
            line_without_newline = line.rstrip('\n\r')
            if line_without_newline.strip() and line_without_newline != line_without_newline.rstrip():
                violations.append(Violation(
                    rule_name="trailing_whitespace",
                    description=self.guidelines["formatting"]["trailing_whitespace"]["rule"],
                    file_path=file_path,
                    line_number=i,
                    line_content=line_without_newline.rstrip(),
                    severity=self.guidelines["formatting"]["trailing_whitespace"]["severity"]
                ))
            
            # Check for tabs instead of spaces
            if '\t' in line:
                violations.append(Violation(
                    rule_name="indentation_tabs",
                    description="Use spaces instead of tabs for indentation",
                    file_path=file_path,
                    line_number=i,
                    line_content=line.rstrip(),
                    severity="warning",
                    suggestion="Replace tabs with 2 spaces"
                ))
            
            # Check space after keywords
            if re.search(self.guidelines["formatting"]["space_after_keywords"]["pattern"], line):
                violations.append(Violation(
                    rule_name="space_after_keywords",
                    description=self.guidelines["formatting"]["space_after_keywords"]["rule"],
                    file_path=file_path,
                    line_number=i,
                    line_content=line.rstrip(),
                    severity=self.guidelines["formatting"]["space_after_keywords"]["severity"],
                    suggestion="Add space between keyword and parenthesis: 'if (condition)'"
                ))
        
        return violations
    
    def _check_naming_conventions(self, file_path: str, lines: List[str]) -> List[Violation]:
        """Check naming convention violations."""
        violations = []
        content = '\n'.join(lines)
        
        # Check class names
        class_pattern = r'class\s+([a-zA-Z_][a-zA-Z0-9_]*)'
        for match in re.finditer(class_pattern, content):
            class_name = match.group(1)
            line_num = content[:match.start()].count('\n') + 1
            
            if line_num <= len(lines) and not re.match(self.guidelines["naming_conventions"]["class_names"]["pattern"], class_name):
                violations.append(Violation(
                    rule_name="class_naming",
                    description=self.guidelines["naming_conventions"]["class_names"]["rule"],
                    file_path=file_path,
                    line_number=line_num,
                    line_content=lines[line_num-1].strip(),
                    severity=self.guidelines["naming_conventions"]["class_names"]["severity"],
                    suggestion=f"Class name '{class_name}' should use PascalCase"
                ))
        
        # Check function names (public methods)
        function_pattern = r'^\s*(?:virtual\s+|static\s+|inline\s+)*(?:const\s+)?[a-zA-Z_][a-zA-Z0-9_<>:]*\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*(?:const\s*)?(?:{|;)'
        for i, line in enumerate(lines, 1):
            match = re.search(function_pattern, line)
            if match and not line.strip().startswith('//'):
                func_name = match.group(1)
                
                # Skip common keywords, operators, and constructors/destructors
                if func_name in ['if', 'for', 'while', 'switch', 'catch', 'return', 'sizeof', 'main'] or func_name.startswith('~'):
                    continue
                
                if not re.match(self.guidelines["naming_conventions"]["function_names"]["pattern"], func_name):
                    violations.append(Violation(
                        rule_name="function_naming",
                        description=self.guidelines["naming_conventions"]["function_names"]["rule"],
                        file_path=file_path,
                        line_number=i,
                        line_content=line.strip(),
                        severity=self.guidelines["naming_conventions"]["function_names"]["severity"],
                        suggestion=f"Function name '{func_name}' should use PascalCase"
                    ))
        
        # Check member variables (look for m_ prefix)
        member_var_pattern = r'^\s*(?:static\s+|const\s+|mutable\s+)*[a-zA-Z_][a-zA-Z0-9_<>:]*\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*[;=]'
        in_class = False
        brace_count = 0
        
        for i, line in enumerate(lines, 1):
            stripped_line = line.strip()
            
            # Track if we're inside a class
            if 'class ' in stripped_line or 'struct ' in stripped_line:
                in_class = True
                brace_count = 0
            
            brace_count += stripped_line.count('{') - stripped_line.count('}')
            
            if in_class and brace_count <= 0 and i > 1:
                in_class = False
            
            # Check member variables inside class
            if in_class and 'private:' in stripped_line:
                continue
            elif in_class and stripped_line and not stripped_line.startswith('//'):
                match = re.search(member_var_pattern, stripped_line)
                if match:
                    var_name = match.group(1)
                    if not var_name.startswith('m_') and not re.match(self.guidelines["naming_conventions"]["member_variables"]["pattern"], var_name):
                        violations.append(Violation(
                            rule_name="member_variable_naming",
                            description=self.guidelines["naming_conventions"]["member_variables"]["rule"],
                            file_path=file_path,
                            line_number=i,
                            line_content=line.strip(),
                            severity=self.guidelines["naming_conventions"]["member_variables"]["severity"],
                            suggestion=f"Member variable '{var_name}' should be prefixed with 'm_'"
                        ))
        
        # Check constants (const variables)
        const_pattern = r'const\s+[a-zA-Z_][a-zA-Z0-9_<>:]*\s+([a-zA-Z_][a-zA-Z0-9_]*)'
        for match in re.finditer(const_pattern, content):
            const_name = match.group(1)
            line_num = content[:match.start()].count('\n') + 1
            
            if line_num <= len(lines) and not re.match(self.guidelines["naming_conventions"]["constant_names"]["pattern"], const_name):
                violations.append(Violation(
                    rule_name="constant_naming",
                    description=self.guidelines["naming_conventions"]["constant_names"]["rule"],
                    file_path=file_path,
                    line_number=line_num,
                    line_content=lines[line_num-1].strip(),
                    severity=self.guidelines["naming_conventions"]["constant_names"]["severity"],
                    suggestion=f"Constant '{const_name}' should use 'k' prefix followed by PascalCase"
                ))
        
        return violations
    
    def _check_best_practices(self, file_path: str, lines: List[str], is_header: bool) -> List[Violation]:
        """Check best practice violations."""
        violations = []
        content = '\n'.join(lines)
        
        # Check for using namespace std in headers
        if is_header:
            for i, line in enumerate(lines, 1):
                if re.search(self.guidelines["best_practices"]["namespace_std_in_headers"]["pattern"], line):
                    violations.append(Violation(
                        rule_name="namespace_usage",
                        description=self.guidelines["best_practices"]["namespace_std_in_headers"]["rule"],
                        file_path=file_path,
                        line_number=i,
                        line_content=line.strip(),
                        severity=self.guidelines["best_practices"]["namespace_std_in_headers"]["severity"],
                        suggestion="Use specific std:: prefixes instead"
                    ))
        
        # Check for raw memory management
        memory_keywords = self.guidelines["best_practices"]["smart_pointers"]["keywords"]
        for i, line in enumerate(lines, 1):
            for keyword in memory_keywords:
                if re.search(rf'\b{keyword}\b', line) and not line.strip().startswith('//'):
                    violations.append(Violation(
                        rule_name="smart_pointers",
                        description=self.guidelines["best_practices"]["smart_pointers"]["rule"],
                        file_path=file_path,
                        line_number=i,
                        line_content=line.strip(),
                        severity=self.guidelines["best_practices"]["smart_pointers"]["severity"],
                        suggestion=self.guidelines["best_practices"]["smart_pointers"]["suggestion"]
                    ))
        
        # Check for NULL/0 instead of nullptr
        for i, line in enumerate(lines, 1):
            if re.search(self.guidelines["best_practices"]["nullptr_usage"]["pattern"], line) and not line.strip().startswith('//'):
                violations.append(Violation(
                    rule_name="nullptr_usage",
                    description=self.guidelines["best_practices"]["nullptr_usage"]["rule"],
                    file_path=file_path,
                    line_number=i,
                    line_content=line.strip(),
                    severity=self.guidelines["best_practices"]["nullptr_usage"]["severity"],
                    suggestion=self.guidelines["best_practices"]["nullptr_usage"]["suggestion"]
                ))
        
        return violations
    
    def _check_code_structure(self, file_path: str, lines: List[str], is_header: bool) -> List[Violation]:
        """Check code structure issues."""
        violations = []
        
        # Check for include guards in headers
        if is_header:
            content = '\n'.join(lines)
            has_pragma_once = '#pragma once' in content
            has_include_guard = re.search(r'#ifndef\s+[A-Z_]+\s*\n.*#define\s+[A-Z_]+', content, re.DOTALL)
            
            if not has_pragma_once and not has_include_guard:
                violations.append(Violation(
                    rule_name="include_guards",
                    description=self.guidelines["code_structure"]["include_guards"]["rule"],
                    file_path=file_path,
                    line_number=1,
                    line_content=lines[0].strip() if lines else "",
                    severity=self.guidelines["code_structure"]["include_guards"]["severity"],
                    suggestion="Add #pragma once at the top or use #ifndef guards"
                ))
        
        # Check function length
        current_function_start = None
        brace_count = 0
        
        for i, line in enumerate(lines, 1):
            stripped_line = line.strip()
            
            # Detect function start
            if re.search(r'^\s*(?:virtual\s+|static\s+|inline\s+)*(?:const\s+)?[a-zA-Z_][a-zA-Z0-9_<>:]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\([^)]*\)\s*(?:const\s*)?{', line):
                current_function_start = i
                brace_count = 1
            elif current_function_start:
                brace_count += stripped_line.count('{') - stripped_line.count('}')
                
                if brace_count == 0:  # Function ended
                    function_length = i - current_function_start + 1
                    if function_length > self.guidelines["code_structure"]["function_length"]["max_lines"]:
                        violations.append(Violation(
                            rule_name="function_length",
                            description=self.guidelines["code_structure"]["function_length"]["rule"],
                            file_path=file_path,
                            line_number=current_function_start,
                            line_content=lines[current_function_start-1].strip(),
                            severity=self.guidelines["code_structure"]["function_length"]["severity"],
                            suggestion=f"Function is {function_length} lines long, consider breaking it down"
                        ))
                    current_function_start = None
        
        return violations
    
    def _check_modern_cpp(self, file_path: str, lines: List[str]) -> List[Violation]:
        """Check modern C++ feature usage."""
        violations = []
        
        # Check lambda default captures
        for i, line in enumerate(lines, 1):
            if re.search(self.guidelines["modern_cpp"]["lambda_captures"]["pattern"], line):
                violations.append(Violation(
                    rule_name="lambda_captures",
                    description=self.guidelines["modern_cpp"]["lambda_captures"]["rule"],
                    file_path=file_path,
                    line_number=i,
                    line_content=line.strip(),
                    severity=self.guidelines["modern_cpp"]["lambda_captures"]["severity"],
                    suggestion=self.guidelines["modern_cpp"]["lambda_captures"]["suggestion"]
                ))
        
        return violations
    
    def _check_comments(self, file_path: str, lines: List[str], is_header: bool) -> List[Violation]:
        """Check comment requirements."""
        violations = []
        
        if is_header:
            # Check for function comments in headers
            for i, line in enumerate(lines, 1):
                if re.search(r'^\s*(?:virtual\s+|static\s+|inline\s+)*(?:const\s+)?[a-zA-Z_][a-zA-Z0-9_<>:]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\([^)]*\)\s*(?:const\s*)?[;{]', line):
                    # Check if previous lines have doxygen comment
                    has_doxygen = False
                    for j in range(max(0, i-5), i):
                        if '/**' in lines[j] or '@brief' in lines[j] or '///' in lines[j]:
                            has_doxygen = True
                            break
                    
                    if not has_doxygen and not line.strip().startswith('//'):
                        violations.append(Violation(
                            rule_name="function_comments",
                            description=self.guidelines["comments"]["function_comments"]["rule"],
                            file_path=file_path,
                            line_number=i,
                            line_content=line.strip(),
                            severity=self.guidelines["comments"]["function_comments"]["severity"],
                            suggestion="Add Doxygen-style comment above function declaration"
                        ))
        
        return violations
    
    def analyze_pr_files(self, changed_files: List[str]) -> List[Violation]:
        """Analyze multiple files (e.g., from a PR)."""
        all_violations = []
        
        for file_path in changed_files:
            if os.path.exists(file_path):
                violations = self.analyze_file(file_path)
                all_violations.extend(violations)
        
        return all_violations
    
    def generate_report(self, violations: List[Violation], format_type: str = "text") -> str:
        """Generate a comprehensive report including violations and AI analysis."""
        if format_type == "json":
            return self._generate_json_report(violations)
        else:
            return self._generate_text_report(violations)
    
    def _generate_text_report(self, violations: List[Violation]) -> str:
        """Generate a comprehensive human-readable text report with AI analysis."""
        report = f"📋 C++ Code Analysis Report\n"
        report += f"{'=' * 50}\n\n"
        
        # Guidelines violations section
        if violations:
            # Group by severity
            errors = [v for v in violations if v.severity == "error"]
            warnings = [v for v in violations if v.severity == "warning"]
            info = [v for v in violations if v.severity == "info"]
            
            report += f"## 📏 Guidelines Compliance\n"
            report += f"  🔴 Errors: {len(errors)}\n"
            report += f"  🟡 Warnings: {len(warnings)}\n"
            report += f"  🔵 Info: {len(info)}\n\n"
            
            # Group by file
            files_violations = {}
            for violation in violations:
                if violation.file_path not in files_violations:
                    files_violations[violation.file_path] = []
                files_violations[violation.file_path].append(violation)
            
            for file_path, file_violations in files_violations.items():
                report += f"### 📁 {file_path}\n"
                report += f"{'-' * (len(file_path) + 6)}\n"
                
                for violation in file_violations:
                    icon = {"error": "🔴", "warning": "🟡", "info": "🔵"}[violation.severity]
                    report += f"{icon} Line {violation.line_number}: {violation.description}\n"
                    report += f"   Code: {violation.line_content[:80]}{'...' if len(violation.line_content) > 80 else ''}\n"
                    if violation.suggestion:
                        report += f"   💡 {violation.suggestion}\n"
                    report += "\n"
        else:
            report += "## 📏 Guidelines Compliance\n"
            report += "✅ No coding guideline violations found!\n\n"
        
        # AI Analysis section
        if self.ai_analyses:
            report += "## 🤖 AI Code Analysis\n"
            report += f"{'=' * 30}\n\n"
            
            for ai_analysis in self.ai_analyses:
                report += f"### 📁 {ai_analysis.file_path}\n"
                report += f"{'-' * (len(ai_analysis.file_path) + 6)}\n"
                
                # Overall scores
                report += f"**Overall Score:** {ai_analysis.overall_score}/10\n"
                report += f"**Maintainability:** {ai_analysis.maintainability_score}/10\n\n"
                
                # Code quality insights
                if ai_analysis.code_quality_insights:
                    report += f"**Code Quality Insights:**\n{ai_analysis.code_quality_insights}\n\n"
                
                # Performance insights
                if ai_analysis.performance_insights:
                    report += f"**Performance Analysis:**\n{ai_analysis.performance_insights}\n\n"
                
                # Improvement suggestions
                if ai_analysis.improvement_suggestions:
                    report += f"**💡 Improvement Suggestions:**\n"
                    for i, suggestion in enumerate(ai_analysis.improvement_suggestions, 1):
                        report += f"  {i}. {suggestion}\n"
                    report += "\n"
                
                # Potential bugs
                if ai_analysis.potential_bugs:
                    report += f"**🐛 Potential Issues:**\n"
                    for i, bug in enumerate(ai_analysis.potential_bugs, 1):
                        report += f"  {i}. {bug}\n"
                    report += "\n"
                
                # Security concerns
                if ai_analysis.security_concerns:
                    report += f"**🔒 Security Concerns:**\n"
                    for i, concern in enumerate(ai_analysis.security_concerns, 1):
                        report += f"  {i}. {concern}\n"
                    report += "\n"
                
                report += "\n"
        
        return report
    
    def _generate_json_report(self, violations: List[Violation]) -> str:
        """Generate a comprehensive JSON report with AI analysis."""
        violations_data = []
        for violation in violations:
            violations_data.append({
                "rule_name": violation.rule_name,
                "description": violation.description,
                "file_path": violation.file_path,
                "line_number": violation.line_number,
                "line_content": violation.line_content,
                "severity": violation.severity,
                "suggestion": violation.suggestion
            })
        
        ai_analyses_data = []
        for analysis in self.ai_analyses:
            ai_analyses_data.append({
                "file_path": analysis.file_path,
                "overall_score": analysis.overall_score,
                "maintainability_score": analysis.maintainability_score,
                "code_quality_insights": analysis.code_quality_insights,
                "improvement_suggestions": analysis.improvement_suggestions,
                "potential_bugs": analysis.potential_bugs,
                "security_concerns": analysis.security_concerns,
                "performance_insights": analysis.performance_insights
            })
        
        return json.dumps({
            "guidelines_analysis": {
                "summary": {
                    "total_violations": len(violations),
                    "errors": len([v for v in violations if v.severity == "error"]),
                    "warnings": len([v for v in violations if v.severity == "warning"]),
                    "info": len([v for v in violations if v.severity == "info"])
                },
                "violations": violations_data
            },
            "ai_analysis": {
                "enabled": self.enable_ai,
                "analyses": ai_analyses_data
            }
        }, indent=2)


def main():
    """Main function to run the analyzer."""
    parser = argparse.ArgumentParser(description="Analyze C++ code for guideline violations with AI insights")
    parser.add_argument("files", nargs="+", help="C++ files to analyze")
    parser.add_argument("--guidelines", help="Custom guidelines JSON file")
    parser.add_argument("--format", choices=["text", "json"], default="text", help="Output format")
    parser.add_argument("--output", help="Output file (default: stdout)")
    parser.add_argument("--groq-api-key", help="Groq API key for AI analysis (or set GROQ_API_KEY env var)")
    parser.add_argument("--disable-ai", action="store_true", help="Disable AI analysis")
    
    args = parser.parse_args()
    
    analyzer = CppGuidelinesAnalyzer(
        guidelines_file=args.guidelines,
        groq_api_key=args.groq_api_key,
        enable_ai=not args.disable_ai
    )
    violations = analyzer.analyze_pr_files(args.files)
    report = analyzer.generate_report(violations, args.format)
    
    if args.output:
        with open(args.output, 'w') as f:
            f.write(report)
        print(f"Report saved to {args.output}")
    else:
        print(report)


if __name__ == "__main__":
    main()
