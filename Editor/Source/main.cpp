#include "EditorApplication.h"

int main() {
    EditorApplication editor;
    
    if (!editor.Initialize()) {
        return -1;
    }
    
    editor.Run();
    editor.Shutdown();
    
    return 0;
}
