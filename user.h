#pragma once
struct user { int uid; char name[32]; bool is_root; };
extern user current_user;
bool user_init();
void login_loop();