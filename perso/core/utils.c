#include <debug.h>
#include <utils.h>

#define C_FOREGROUND C_BLUE
#define C_BACKGROUND S_RST
void print_secos(void) {
    /* const char secos[] = "█\n╗\n╝\n═\n╚\n╔\n║\n"; */
    const char secos[] = "\n"
        "\t███████╗███████╗ ██████╗ ██████╗ ███████╗\n"
        "\t██╔════╝██╔════╝██╔════╝██╔═══██╗██╔════╝\n"
        "\t███████╗█████╗  ██║     ██║   ██║███████╗\n"
        "\t╚════██║██╔══╝  ██║     ██║   ██║╚════██║\n"
        "\t███████║███████╗╚██████╗╚██████╔╝███████║\n"
        "\t╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝\n";

    for(uint32_t i = 0; i < sizeof(secos);) {
        if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -106 && secos[i + 2] == -120) {
            debug_color(C_FOREGROUND, "█");
            i += 2;
        } else if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -107 && secos[i + 2] == -105) {
            debug_color(C_BACKGROUND, "╗");
            i += 2;
        } else if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -107 && secos[i + 2] == -99) {
            debug_color(C_BACKGROUND, "╝");
            i += 2;
        } else if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -107 && secos[i + 2] == -112) {
            debug_color(C_BACKGROUND, "═");
            i += 2;
        } else if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -107 && secos[i + 2] == -102) {
            debug_color(C_BACKGROUND, "╚");
            i += 2;
        } else if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -107 && secos[i + 2] == -108) {
            debug_color(C_BACKGROUND, "╔");
            i += 2;
        } else if(i + 2 < sizeof(secos) && secos[i] == -30 && secos[i + 1] == -107 && secos[i + 2] == -111) {
            debug_color(C_BACKGROUND, "║");
            i += 2;
        } else {
            debug("%c", secos[i++]);
        }
    }
}

void print_stack(uint32_t ebp, uint32_t esp) {
    debug("\n### ESP:\n");
    while(esp < ebp) {
        debug("%p: %p\n", esp, *(uint32_t*)esp);
        esp += 4;
    }
    debug ("### EBP (%p)\n", ebp);
}
