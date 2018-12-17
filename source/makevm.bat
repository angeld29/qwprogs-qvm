@echo off
rem make sure we have a safe environement
set LIBRARY=
set INCLUDE=

mkdir vm
cd vm
set cc=lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\ %1

%cc%  ../bg_lib.c
@if errorlevel 1 goto quit
%cc%  ../buttons.c
@if errorlevel 1 goto quit
%cc%  ../client.c
@if errorlevel 1 goto quit
%cc%  ../combat.c
@if errorlevel 1 goto quit
%cc%  ../doors.c
@if errorlevel 1 goto quit
%cc%  ../g_main.c
@if errorlevel 1 goto quit
%cc%  ../g_cmd.c
@if errorlevel 1 goto quit
%cc%  ../g_mem.c
@if errorlevel 1 goto quit
%cc%  ../g_spawn.c
@if errorlevel 1 goto quit
%cc%  ../g_utils.c
@if errorlevel 1 goto quit
%cc%  ../items.c
@if errorlevel 1 goto quit
%cc%  ../mathlib.c
@if errorlevel 1 goto quit
%cc%  ../misc.c
@if errorlevel 1 goto quit
%cc%  ../plats.c
@if errorlevel 1 goto quit
%cc%  ../player.c
@if errorlevel 1 goto quit
%cc%  ../q_shared.c
@if errorlevel 1 goto quit
%cc%  ../server.c
@if errorlevel 1 goto quit
%cc%  ../spectate.c
@if errorlevel 1 goto quit
%cc%  ../subs.c
@if errorlevel 1 goto quit
%cc%  ../triggers.c
@if errorlevel 1 goto quit
%cc%  ../weapons.c
@if errorlevel 1 goto quit
%cc%  ../world.c
@if errorlevel 1 goto quit
%cc%  ../g_mod_command.c
@if errorlevel 1 goto quit
    
q3asm -f ../game
:quit
cd ..


exit