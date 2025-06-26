using namespace std;  // Bad practice in header

class myBadClass {  // Should be PascalCase
public:
    int* getRawPointer();  // Should avoid raw pointers
    void SomeMethod();     // Should use camelCase or snake_case
    
private:
    int MEMBER_VAR;        // Should use camelCase or m_ prefix
};

void AnotherFunctionWithVeryLongNameThatExceedsTheLineLimit(int param1, int param2, int param3, int param4);

const int globalConstant = 42;  // Should be UPPER_SNAKE_CASE
