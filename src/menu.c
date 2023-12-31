#include "angband.h"
#include "menu.h"

void default_menu(int cmd, int which, vptr cookie, var_ptr res)
{
    var_clear(res);
}

static void _describe(menu_ptr menu, int which)
{
    var_t v = var_create();

    menu->fn(MENU_HELP, which, menu->cookie, &v);
    if (!var_is_null(&v))
    {
        char tmp[255*10];
        int i, line;

        for (i = 0; i < 2+10; i++)
            Term_erase(13, menu->count + i + 1 + (menu->heading ? 1 : 0), 255);

        roff_to_buf(var_get_string(&v), 80-15, tmp, sizeof(tmp));

        for(i = 0, line = menu->count + 2 + (menu->heading ? 1 : 0); tmp[i]; i += 1+strlen(&tmp[i]))
        {
            prt(&tmp[i], line, 15);
            line++;
        }
    }
    var_destroy(&v);
}

static void _list(menu_ptr menu, char *keys)
{
    char temp[255];
    int  i;
    int  y = 1;
    int  x = 0;

    Term_erase(x, y, 255);

    if (menu->heading)
        put_str(format("    %s", menu->heading), y++, x);

    for (i = 0; i < menu->count; i++)
    {
        byte attr = TERM_WHITE;
        var_t key = var_create();
        var_t text = var_create();
        var_t color = var_create();

        menu->fn(MENU_KEY, i, menu->cookie, &key);
        if (var_is_null(&key))
            keys[i] = I2A(i);
        else
            keys[i] = (char)var_get_int(&key);

        if (menu->count <= 26)
            keys[i] = tolower(keys[i]);

        menu->fn(MENU_TEXT, i, menu->cookie, &text);
        if (var_is_null(&text))
            var_set_string(&text, "");

        menu->fn(MENU_COLOR, i, menu->cookie, &color);
        if (!var_is_null(&color))
            attr = var_get_int(&color);

        if (attr == TERM_DARK)
            keys[i] = '\0';

        sprintf(temp, " %c) %s", keys[i], var_get_string(&text));
        c_prt(attr, temp, y + i, x);

        var_destroy(&key);
        var_destroy(&text);
        var_destroy(&color);
    }
    Term_erase(x, y + menu->count, 255);
}

static int _choose(menu_ptr menu)
{
    int  choice = -1;
    bool describe = FALSE;
    bool allow_browse = FALSE;
    char choose_prompt[255];
    char browse_prompt[255];
    char keys[100];
    
    if (menu->browse_prompt)
    {
        allow_browse = TRUE;
        sprintf(choose_prompt, "%s (Type '?' to browse)", menu->choose_prompt);
        sprintf(browse_prompt, "%s (Type '?' to choose)", menu->browse_prompt);
    }
    else
    {
        sprintf(choose_prompt, "%s", menu->choose_prompt);
        sprintf(browse_prompt, "%s", "");
    }
    
    _list(menu, keys);

    for (;;)
    {
        char ch = '\0';
        int i;

        choice = -1;
        if (!get_com(describe ? browse_prompt : choose_prompt, &ch, FALSE)) break;

        if (ch == '?' && allow_browse)
        {
            describe = !describe;
            continue;
        }

        for (i = 0; i < menu->count; i++)
        {
            if (menu->count <= 26)
            {
                if (toupper(keys[i]) == toupper(ch))
                {
                    choice = i;
                    break;
                }
            }
            else if (keys[i] == ch)
            {
                choice = i;
                break;
            }
        }

        if (choice < 0 || choice >= menu->count)
        {
            bell();
            continue;
        }

        if (describe)
        {
            _describe(menu, choice);
            continue;
        }

        break;
    }
    return choice;
}

int menu_choose(menu_ptr menu)
{
    int choice = -1;

    if (REPEAT_PULL(&choice))
    {
        if (choice >= 0 && choice < menu->count)
            return choice;
    }

    screen_save();

    choice = _choose(menu);
    REPEAT_PUSH(choice);

    screen_load();

    return choice;
}
