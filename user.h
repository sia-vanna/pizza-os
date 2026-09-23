#pragma once
#include <stdint.h>

struct user {
  uint32_t uid;
  char name[32];
  bool is_root;
};

extern user current_user;
bool user_init();
void login_loop();