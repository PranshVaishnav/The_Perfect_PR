#!/usr/bin/env python3
"""
Demo script to showcase AI-enhanced C++ code analysis
Run this script to test the integration with Groq AI
"""

import os
from cpp_code_analyzer import CppGuidelinesAnalyzer

def main():
    """Demo the AI-enhanced C++ analyzer."""
    print("🚀 C++ Code Analyzer with AI Integration Demo")
    print("=" * 50)
    
    # Check if Groq API key is available
    api_key = os.getenv('GROQ_API_KEY')
    if not api_key:
        print("⚠️  GROQ_API_KEY not found in environment variables.")
        print("   Set your API key with: export GROQ_API_KEY='your-api-key-here'")
        print("   Or pass it as --groq-api-key parameter when running the analyzer.")
        print("\n   Running with AI analysis disabled...\n")
        enable_ai = False
    else:
        print(f"✅ Found Groq API key: {api_key[:10]}...{api_key[-4:]}")
        enable_ai = True
    
    # Initialize analyzer
    analyzer = CppGuidelinesAnalyzer(enable_ai=enable_ai)
    
    # Check if sample files exist
    sample_files = ['sample_code.cpp', 'sample_header.h']
    existing_files = [f for f in sample_files if os.path.exists(f)]
    
    if not existing_files:
        print("❌ No sample C++ files found!")
        print("   Please ensure 'sample_code.cpp' or 'sample_header.h' exist in the current directory.")
        return
    
    print(f"📁 Analyzing {len(existing_files)} file(s): {', '.join(existing_files)}")
    print()
    
    # Analyze files
    all_violations = []
    for file_path in existing_files:
        print(f"🔍 Analyzing {file_path}...")
        violations = analyzer.analyze_file(file_path)
        all_violations.extend(violations)
        print(f"   Found {len(violations)} guideline violations")
    
    print(f"\n📊 Analysis Summary:")
    print(f"   Total violations: {len(all_violations)}")
    print(f"   AI analyses: {len(analyzer.ai_analyses)}")
    
    # Generate and display report
    print("\n" + "=" * 50)
    print("📋 DETAILED REPORT")
    print("=" * 50)
    
    report = analyzer.generate_report(all_violations, "text")
    print(report)
    
    # Save report to file
    with open('analysis_report.txt', 'w') as f:
        f.write(report)
    print("\n💾 Report saved to 'analysis_report.txt'")
    
    # Demo JSON output
    json_report = analyzer.generate_report(all_violations, "json")
    with open('analysis_report.json', 'w') as f:
        f.write(json_report)
    print("💾 JSON report saved to 'analysis_report.json'")

if __name__ == "__main__":
    main() 