#include <stdlib.h>
#include "../exceptions.h"

// A function that throws an exception outside of any TRY block
void throw_outside_try() {
    THROW(3, "This exception has no TRY block!");  // Will trigger abort()
}

int main() {
    TRY{
        throw_outside_try();
    }
    CATCH(3){
        printf("Catched!");
    }
    CATCH_ALL{
        printf("CATCHEDDDDDDDDDDD");
    }
    END_TRY;
    return 0;
}
