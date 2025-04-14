#include "../includes/auth.h"    // Correct path
#include "../includes/db.h"      // Correct path
#include "../includes/queries.h" // Correct path
#include <stdio.h>
#include <stdlib.h> // For exit()
#include <string.h> // For strcmp()

// Function prototypes for menus
void show_admin_menu(UserSession *session);
void show_broker_menu(UserSession *session);

int main() {
  const char *db_path = "ParfumeMarket.db"; // Relative path
  const char *schema_path = "database_schema.sql";

  // 1. Open Database
  if (open_db(db_path) != 0) {
    fprintf(stderr, "Failed to open database '%s'. Exiting.\n", db_path);
    return 1;
  }

  // 2. Initialize Tables if needed
  if (init_tables_if_needed(schema_path) != 0) {
    fprintf(stderr,
            "Failed to initialize database schema from '%s'. Exiting.\n",
            schema_path);
    close_db();
    return 1;
  }

  // 3. Authentication
  UserSession current_session;
  memset(&current_session, 0, sizeof(UserSession)); // Clear session info
  int login_attempts = 0;
  const int MAX_LOGIN_ATTEMPTS = 3;

  while (!current_session.is_authenticated &&
         login_attempts < MAX_LOGIN_ATTEMPTS) {
    char username[MAX_USERNAME_LEN];
    char password[100]; // Adjust size as needed

    printf("\n--- Аутентификация ---\n");
    printf("Попытка %d из %d\n", login_attempts + 1, MAX_LOGIN_ATTEMPTS);

    // Use safe_scanf or similar secure input method here
    printf("Имя пользователя: ");
    scanf("%49s", username); // Limit input size (still basic protection)
    printf("Пароль: ");
    scanf("%99s", password); // Limit input size
    // Consume potential leftover newline
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
      ;

    // --- !!! WARNING: PASSING PLAINTEXT PASSWORD !!! ---
    int login_result = login_user(username, password, &current_session);
    // Clear password from memory ASAP
    memset(password, 0, sizeof(password));

    if (login_result == 0) {
      // Login successful - break loop
      break;
    } else if (login_result == 1) {
      // Login failed (credentials)
      login_attempts++;
    } else {
      // Database or other error during login
      fprintf(stderr, "Login error. Exiting.\n");
      close_db();
      return 1;
    }
  }

  if (!current_session.is_authenticated) {
    printf("Превышено количество попыток входа.\n");
    close_db();
    return 1;
  }

  // 4. Authorization & Menu Display
  if (strcmp(current_session.role, "admin") == 0) {
    show_admin_menu(&current_session);
  } else if (strcmp(current_session.role, "broker") == 0) {
    show_broker_menu(&current_session);
  } else {
    printf("Неизвестная роль пользователя: %s\n", current_session.role);
  }

  // 5. Close Database
  close_db();
  printf("Программа завершена.\n");
  return 0;
}

// --- Admin Menu ---
void show_admin_menu(UserSession *session) {
  int choice;
  do {
    printf("\n=== Меню Администратора (%s) ===\n", session->username);
    printf("--- Запросы (Task 2) ---\n");
    printf(" 1. Продажи за период\n");
    printf(" 2. Покупатели по товару (опц. фильтр)\n");
    printf(" 3. Инфо по самому популярному типу товара\n");
    printf(" 4. Маклер с макс. количеством сделок\n");
    printf(" 5. Маклеры по поставщикам (опц. фильтр)\n");
    printf("--- Управление данными (Task 3) ---\n");
    printf(" 10. Добавить нового маклера\n");
    printf(" 11. Добавить новый товар\n");
    printf(" 12. Добавить новую сделку\n");
    printf(" 13. Обновить цену товара\n");
    printf(" 14. Удалить сделку по ID\n");
    // Add more CRUD options: Suppliers, Buyers, Users
    printf("--- Функции (Task 4, 5, 6) ---\n");
    printf(" 20. Пересчитать статистику маклеров (Task 4 - Batch)\n");
    printf(" 21. Обновить остатки и очистить сделки до даты (Task 5)\n");
    printf(" 22. Показать сделки на указанную дату (Task 6)\n");
    printf("---------------------------\n");
    printf(" 0. Выход\n");

    choice = safe_scanf_int("Ваш выбор: "); // Use safer input

    switch (choice) {
    // Task 2
    case 1:
      run_sales_summary_by_period();
      break;
    case 2:
      run_buyers_by_good();
      break;
    case 3:
      run_most_popular_type_info();
      break; // (*) Accessible to admin
    case 4:
      run_top_broker_info();
      break;
    case 5:
      run_supplier_brokers_info();
      break;
    // Task 3
    case 10:
      add_new_broker();
      break;
    case 11:
      add_new_good();
      break;
    case 12:
      add_new_deal();
      break;
    case 13:
      update_good_price();
      break;
    case 14:
      delete_deal_by_id();
      break;
    // Task 4, 5, 6
    case 20:
      recalculate_broker_stats();
      break; // (*) Accessible to admin
    case 21:
      update_goods_quantity_and_clear_deals();
      break;
    case 22:
      show_deals_on_date();
      break; // (*) Accessible to admin

    case 0:
      printf("Выход из меню администратора...\n");
      break;
    default:
      printf("Неверный пункт меню!\n");
      break;
    }
  } while (choice != 0);
}

// --- Broker Menu ---
void show_broker_menu(UserSession *session) {
  int choice;
  if (strlen(session->broker_surname) == 0) {
    printf("Ошибка: Не удалось определить фамилию маклера для сессии.\n");
    return;
  }

  do {
    printf("\n=== Меню Маклера (%s - %s) ===\n", session->username,
           session->broker_surname);
    printf("--- Доступные операции ---\n");
    printf(" 1. Показать мои сделки\n");
    printf(
        " 2. Покупатели по товару (все товары)\n"); // Example of shared query
    printf(" 3. Инфо по самому популярному типу товара (*)\n"); // Task 2*
    // Maybe add ability to add a deal *for themselves*?
    // printf(" 4. Добавить новую сделку (для себя)\n");
    printf("---------------------------\n");
    printf(" 0. Выход\n");

    choice = safe_scanf_int("Ваш выбор: ");

    switch (choice) {
    case 1:
      show_broker_deals(session->broker_surname);
      break;
    case 2:
      run_buyers_by_good();
      break; // Allow broker to see general buyer info
    case 3:
      run_most_popular_type_info();
      break; // (*) As per requirement
    // case 4: // Add function call for broker adding their own deal
    case 0:
      printf("Выход из меню маклера...\n");
      break;
    default:
      printf("Неверный пункт меню!\n");
      break;
    }
    // Tasks 4, 5, 6 (marked with * or general) are typically admin functions
    // Task 4 (*): Broker doesn't trigger recalc, maybe view their own stats?
    // Task 5: Admin function
    // Task 6 (*): Broker can view deals, maybe filter by date *and* their
    // surname?

  } while (choice != 0);
}