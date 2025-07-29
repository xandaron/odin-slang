#include "slangc.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Testing Slang C Wrapper...\n");
    
    // Create global session
    SlangcGlobalSession* globalSession = NULL;
    SlangcResult result = slangc_createGlobalSession(&globalSession);
    
    if (result != SLANGC_OK) {
        printf("Error: Failed to create global session (code: %d)\n", result);
        return 1;
    }
    
    printf("Success: Global session created!\n");
    
    // Clean up
    slangc_releaseGlobalSession(globalSession);
    
    printf("Test completed successfully!\n");
    return 0;
}
