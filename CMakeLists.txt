# Проверка версии CMake
cmake_minimum_required( VERSION 3.4.0 )
# Если версия установленой программы ниже, то ошибка выполнения

# Название проекта и проверка доступности компиляторя с++
project( qwprogs C )			

# Установка переменной со списком исходников
set( SOURCE_LIB 
    "source/buttons.c"
    "source/client.c"
    "source/combat.c"
    "source/doors.c"
    "source/g_cmd.c"
    "source/g_main.c"
    "source/g_mem.c"
    "source/g_mod_command.c"
    "source/g_spawn.c"
    "source/g_syscalls.c"
    "source/g_utils.c"
    "source/items.c"
    "source/mathlib.c"
    "source/misc.c"
    "source/plats.c"
    "source/player.c"
    "source/q_shared.c"
    "source/server.c"
    "source/spectate.c"
    "source/subs.c"
    "source/triggers.c"
    "source/weapons.c"
    "source/world.c"
)		

# Включение экспорта всех символов для создания динамической библиотеки
set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )

# Создание динамической библиотеки с именем example
add_library( qwprogs SHARED ${SOURCE_LIB} )	