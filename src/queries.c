#include "../includes/queries.h" // Correct path
#include "../includes/db.h"      // Correct path
#include <stdio.h> // <<< Make sure this is included for printf, fgets, etc.
#include <stdlib.h>
#include <string.h>

// --- Helper for safe string input ---
// WARNING: Basic - real applications need more robust input handling
void safe_scanf(const char *prompt, char *buffer, size_t buffer_size) {
  printf("%s", prompt);
  if (fgets(buffer, buffer_size, stdin) != NULL) {
    // Remove trailing newline, if exists
    buffer[strcspn(buffer, "\n")] = 0;
  } else {
    // Handle error or EOF
    buffer[0] = '\0';
  }
  // Clear remaining input buffer if needed
  if (strchr(buffer, '\0')[-1] != '\n' && strlen(buffer) == buffer_size - 1) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
      ;
  }
}
// --- Helper for safe int input ---
int safe_scanf_int(const char *prompt) {
  char buffer[50];
  int value = 0;      // Default or error value
  long long temp_val; // Use long long for wider range check
  char *endptr;

  while (1) {
    safe_scanf(prompt, buffer, sizeof(buffer));
    if (buffer[0] == '\0')
      continue; // Empty input

    temp_val = strtoll(buffer, &endptr, 10);

    // Check if conversion was successful and covered the entire string
    if (endptr == buffer || *endptr != '\0') {
      printf("Invalid input. Please enter a number.\n");
    } else if (temp_val < -2147483647 ||
               temp_val > 2147483647) { // Check range for int
      printf("Input out of range for an integer.\n");
    } else {
      value = (int)temp_val;
      break; // Valid input
    }
  }
  return value;
}

// --- Task 2 Queries ---

void run_sales_summary_by_period() {
  char start[11], end[11];
  // Consider adding input validation for date format
  safe_scanf("Начальная дата (YYYY-MM-DD): ", start, sizeof(start));
  safe_scanf("Конечная дата (YYYY-MM-DD): ", end, sizeof(end));

  char query[1024];
  // Corrected JOIN condition and aliases for clarity
  snprintf(
      query, sizeof(query),
      "SELECT d.good_name_fk AS GoodName, SUM(d.sell_quantity) AS TotalSold, "
      "SUM(d.sell_quantity * g.price) AS TotalIncome "
      "FROM Deals d JOIN Goods g ON d.good_name_fk = g.name AND "
      "d.supplier_name_fk = g.supplier_name_fk "
      "WHERE d.deal_date BETWEEN '%s' AND '%s' GROUP BY d.good_name_fk;",
      start, end); // TODO: Use prepared statements to prevent SQL injection

  execute_select_query(query);
}

void run_buyers_by_good() {
  char good_name_filter[100];
  safe_scanf(
      "Введите название товара для фильтрации (оставьте пустым для всех): ",
      good_name_filter, sizeof(good_name_filter));

  char query[1024];
  char where_clause[150] = "";
  if (strlen(good_name_filter) > 0) {
    snprintf(where_clause, sizeof(where_clause), "WHERE d.good_name_fk = '%s'",
             good_name_filter); // SQL Injection risk! Use prepared statements.
  }

  // Corrected JOIN and added aliases
  snprintf(query, sizeof(query),
           "SELECT d.good_name_fk AS GoodName, d.buyer_name_fk AS Buyer, "
           "SUM(d.sell_quantity) AS TotalUnits, "
           "SUM(d.sell_quantity * g.price) AS TotalCost "
           "FROM Deals d JOIN Goods g ON d.good_name_fk = g.name AND "
           "d.supplier_name_fk = g.supplier_name_fk "
           "%s " // Insert WHERE clause here
           "GROUP BY d.good_name_fk, d.buyer_name_fk ORDER BY GoodName, Buyer;",
           where_clause);
  execute_select_query(query);
}

void run_most_popular_type_info() {
  // Find the most popular type first
  // NOTE: This requires more complex logic or potentially a better data model
  // if 'type_of_good' needs to be consistently tracked and queried efficiently.
  // The original query assumes 'type_of_good' is reliably in Deals.
  // Let's stick to the original logic for now, assuming the Deals table has the
  // type.

  const char
      *query = "WITH TypeSales AS ("
               "  SELECT type_of_good, SUM(sell_quantity) AS total_sold "
               "  FROM Deals WHERE type_of_good IS NOT NULL GROUP BY "
               "type_of_good " // Added check for NULL
               "), MaxType AS ("
               "  SELECT type_of_good FROM TypeSales "
               "  ORDER BY total_sold DESC LIMIT 1" // Simplified way to get max
               ") "
               "SELECT d.buyer_name_fk AS Buyer, d.type_of_good AS GoodType, "
               "SUM(d.sell_quantity) AS TotalUnits, "
               "SUM(d.sell_quantity * g.price) AS TotalCost "
               "FROM Deals d "
               "JOIN Goods g ON d.good_name_fk = g.name AND d.supplier_name_fk "
               "= g.supplier_name_fk "
               "WHERE d.type_of_good = (SELECT type_of_good FROM MaxType) "
               "GROUP BY d.buyer_name_fk, d.type_of_good ORDER BY Buyer;"; // Added
                                                                           // ORDER
                                                                           // BY
  printf("--- Информация по самому популярному типу товара ---\n");
  execute_select_query(query);
}

void run_top_broker_info() {
  const char *query =
      "WITH BrokerDeals AS ("
      "  SELECT broker_surname_fk, COUNT(*) AS deal_count "
      "  FROM Deals GROUP BY broker_surname_fk"
      "), TopBroker AS ("
      "  SELECT broker_surname_fk FROM BrokerDeals "
      "  ORDER BY deal_count DESC LIMIT 1" // Simplified max
      ") "
      // Select broker details and unique suppliers they dealt with
      "SELECT b.surname, b.address, b.birth_year, GROUP_CONCAT(DISTINCT "
      "d.supplier_name_fk) AS Suppliers "
      "FROM Brokers b "
      "JOIN Deals d ON b.surname = d.broker_surname_fk "
      "WHERE b.surname = (SELECT broker_surname_fk FROM TopBroker) "
      "GROUP BY b.surname, b.address, b.birth_year;"; // Group by broker details
  printf("--- Информация о Маклере с максимальным количеством сделок ---\n");
  execute_select_query(query);
}

void run_supplier_brokers_info() {
  char supplier_filter[100];
  safe_scanf("Введите название фирмы-поставщика для фильтрации (оставьте "
             "пустым для всех): ",
             supplier_filter, sizeof(supplier_filter));

  char query[1024];
  char where_clause[150] = "";
  if (strlen(supplier_filter) > 0) {
    snprintf(where_clause, sizeof(where_clause),
             "WHERE d.supplier_name_fk = '%s'",
             supplier_filter); // SQL Injection risk!
  }

  // Corrected JOIN, added aliases and ORDER BY
  snprintf(query, sizeof(query),
           "SELECT d.supplier_name_fk AS Supplier, d.broker_surname_fk AS "
           "Broker, SUM(d.sell_quantity) AS TotalSold, "
           "SUM(d.sell_quantity * g.price) AS TotalValue "
           "FROM Deals d "
           "JOIN Goods g ON d.good_name_fk = g.name AND d.supplier_name_fk = "
           "g.supplier_name_fk "
           "%s " // Insert WHERE clause
           "GROUP BY d.supplier_name_fk, d.broker_surname_fk "
           "ORDER BY Supplier, Broker;",
           where_clause);
  printf("--- Информация о маклерах по поставщикам ---\n");
  execute_select_query(query);
}

// --- Task 3 CRUD Operations ---

void add_new_broker() {
  char surname[100], address[200];
  int birth_year;
  safe_scanf("Фамилия маклера: ", surname, sizeof(surname));
  safe_scanf("Адрес маклера: ", address, sizeof(address));
  birth_year = safe_scanf_int("Год рождения: ");

  // TODO: Add checks if broker already exists?

  char query[512];
  // Use prepared statements in real code!
  snprintf(query, sizeof(query),
           "INSERT INTO Brokers (surname, address, birth_year) VALUES ('%s', "
           "'%s', %d);",
           surname, address, birth_year);

  if (execute_non_query(query) == SQLITE_OK) {
    printf("Маклер '%s' успешно добавлен.\n", surname);
  } else {
    printf("Не удалось добавить маклера.\n");
  }
}

void add_new_good() {
  char name[100], type[100], supplier[100], expiry[11];
  double price;
  int quantity;

  safe_scanf("Название товара: ", name, sizeof(name));
  safe_scanf("Вид (тип) товара: ", type, sizeof(type));
  safe_scanf("Фирма-поставщик: ", supplier,
             sizeof(supplier)); // Check if supplier exists?
  // Using safe_scanf_int for price now, assuming integer price for simplicity
  // If double is needed, create safe_scanf_double or use sscanf carefully
  price = (double)safe_scanf_int("Цена за единицу (целое число): ");
  quantity = safe_scanf_int("Количество поставленных единиц: ");
  safe_scanf("Срок годности (YYYY-MM-DD, оставьте пустым если нет): ", expiry,
             sizeof(expiry));

  // TODO: Check if supplier exists in Suppliers table first. Add supplier if
  // not?
  // TODO: Check if this good from this supplier already exists? Update quantity
  // instead?

  char query[1024];
  char expiry_sql[20];
  if (strlen(expiry) > 0) {
    snprintf(expiry_sql, sizeof(expiry_sql), "'%s'", expiry);
  } else {
    strcpy(expiry_sql, "NULL");
  }

  // Prepared statements are essential here
  snprintf(query, sizeof(query),
           "INSERT INTO Goods (name, type_of_good, price, supplier_name_fk, "
           "expiry_date, quantity) "
           "VALUES ('%s', '%s', %.2f, '%s', %s, %d);",
           name, type, price, supplier, expiry_sql, quantity);

  if (execute_non_query(query) == SQLITE_OK) {
    printf("Товар '%s' от '%s' успешно добавлен.\n", name, supplier);
  } else {
    printf("Не удалось добавить товар.\n");
  }
}

void add_new_deal() {
  char date[11], good_name[100], supplier[100], broker[100], buyer[100],
      type[100];
  int quantity;

  safe_scanf("Дата сделки (YYYY-MM-DD): ", date, sizeof(date));
  safe_scanf("Название товара: ", good_name, sizeof(good_name));
  safe_scanf("Фирма-поставщик товара: ", supplier, sizeof(supplier));
  safe_scanf("Вид (тип) товара: ", type,
             sizeof(type)); // Could fetch from Goods table?
  quantity = safe_scanf_int("Количество проданных единиц: ");
  safe_scanf("Фамилия маклера: ", broker,
             sizeof(broker)); // Check if broker exists?
  safe_scanf("Фирма-покупатель: ", buyer,
             sizeof(buyer)); // Check if buyer exists? Add if not?

  // --- Important Checks Needed ---
  // 1. Does the Good exist (name, supplier)?
  // 2. Is there enough quantity in the Goods table?
  // 3. Does the Broker exist?
  // 4. Does the Buyer exist? (Add if not?)
  // 5. Update Goods quantity after successful deal (Transaction needed)
  // 6. Update BrokerStats after successful deal (Task 4)
  // --------------------------------

  // Example Query (Needs checks and transactions)
  char query[1024];
  snprintf(query, sizeof(query),
           "INSERT INTO Deals (deal_date, good_name_fk, supplier_name_fk, "
           "type_of_good, sell_quantity, broker_surname_fk, buyer_name_fk) "
           "VALUES ('%s', '%s', '%s', '%s', %d, '%s', '%s');",
           date, good_name, supplier, type, quantity, broker, buyer);

  // --- Wrap INSERT Deal and UPDATE Goods in a TRANSACTION ---
  execute_non_query("BEGIN TRANSACTION;");

  if (execute_non_query(query) == SQLITE_OK) {
    // Now update the Goods quantity
    char update_goods_query[512];
    snprintf(update_goods_query, sizeof(update_goods_query),
             "UPDATE Goods SET quantity = quantity - %d WHERE name = '%s' AND "
             "supplier_name_fk = '%s' AND quantity >= %d;",
             quantity, good_name, supplier, quantity);

    if (execute_non_query(update_goods_query) == SQLITE_OK &&
        sqlite3_changes(db) > 0) {
      // Now potentially update BrokerStats (Task 4) - complex part
      // update_single_broker_stat(broker, quantity, price_of_good); //
      // Hypothetical function

      execute_non_query("COMMIT;");
      printf("Сделка успешно добавлена и остатки обновлены.\n");
      // Consider calling recalculate_broker_stats() here if incremental is too
      // complex
      recalculate_broker_stats(); // Run batch update for simplicity for now
    } else {
      execute_non_query("ROLLBACK;");
      if (sqlite3_changes(db) == 0) {
        printf("Не удалось добавить сделку: Недостаточно товара '%s' от '%s' "
               "на складе или товар не найден.\n",
               good_name, supplier);
      } else {
        printf("Не удалось добавить сделку: Ошибка при обновлении остатков.\n");
      }
    }
  } else {
    execute_non_query("ROLLBACK;");
    printf(
        "Не удалось добавить сделку: Ошибка при добавлении записи в Deals.\n");
  }
}

void update_good_price() {
  char name[100], supplier[100];
  double new_price;

  safe_scanf("Название товара для обновления цены: ", name, sizeof(name));
  safe_scanf("Фирма-поставщик товара: ", supplier, sizeof(supplier));
  new_price = (double)safe_scanf_int(
      "Новая цена за единицу (целое число): "); // Using safe int input

  if (new_price <= 0) {
    printf("Цена должна быть положительной.\n");
    return;
  }

  char query[512];
  snprintf(query, sizeof(query),
           "UPDATE Goods SET price = %.2f WHERE name = '%s' AND "
           "supplier_name_fk = '%s';",
           new_price, name, supplier); // SQL Injection risk

  if (execute_non_query(query) == SQLITE_OK) {
    if (sqlite3_changes(db) > 0) { // Check if any row was actually updated
      printf("Цена товара '%s' от '%s' успешно обновлена.\n", name, supplier);
    } else {
      printf("Товар '%s' от '%s' не найден.\n", name, supplier);
    }
  } else {
    printf("Не удалось обновить цену товара.\n");
  }
}

void delete_deal_by_id() {
  int deal_id = safe_scanf_int("Введите ID сделки для удаления: ");

  // Optional: Ask for confirmation

  // --- Consideration ---
  // Should deleting a deal revert the Goods quantity?
  // If yes, need to fetch deal details before deleting and wrap in transaction.
  // For simplicity now, just delete the record.
  // ---------------------

  char query[256];
  snprintf(query, sizeof(query), "DELETE FROM Deals WHERE deal_id = %d;",
           deal_id);

  if (execute_non_query(query) == SQLITE_OK) {
    if (sqlite3_changes(db) > 0) {
      printf("Сделка с ID %d успешно удалена.\n", deal_id);
      // Potentially recalculate broker stats if needed
    } else {
      printf("Сделка с ID %d не найдена.\n", deal_id);
    }
  } else {
    printf("Не удалось удалить сделку.\n");
  }
}

// --- Task 4, 5, 6 Functions ---

// Task 4: Recalculate stats for ALL brokers and update BrokerStats table
// Note: This is a batch update, not the incremental update potentially implied
// by Task 4.
void recalculate_broker_stats() {
  printf("Пересчет статистики маклеров...\n");
  // Clear existing stats or use INSERT OR REPLACE / UPDATE
  // Using separate DELETE + INSERT for simplicity here
  execute_non_query("BEGIN TRANSACTION;");

  const char *delete_query = "DELETE FROM BrokerStats;";
  if (execute_non_query(delete_query) != SQLITE_OK) {
    execute_non_query("ROLLBACK;");
    printf("Ошибка при очистке статистики маклеров.\n");
    return;
  }

  // Insert updated stats
  const char *insert_query =
      "INSERT INTO BrokerStats (broker_surname_fk, total_sold_units, "
      "total_deal_sum, last_updated) "
      "SELECT "
      "  d.broker_surname_fk, "
      "  SUM(d.sell_quantity), "
      "  SUM(d.sell_quantity * g.price), "
      "  datetime('now', 'localtime') "
      "FROM Deals d JOIN Goods g ON d.good_name_fk = g.name AND "
      "d.supplier_name_fk = g.supplier_name_fk "
      "GROUP BY d.broker_surname_fk;";

  if (execute_non_query(insert_query) == SQLITE_OK) {
    execute_non_query("COMMIT;");
    printf("Статистика маклеров успешно обновлена (пакетно).\n");
    // Optional: Display the updated stats
    execute_select_query(
        "SELECT bs.*, b.address, b.birth_year FROM BrokerStats bs JOIN Brokers "
        "b ON bs.broker_surname_fk = b.surname;");

  } else {
    execute_non_query("ROLLBACK;");
    printf("Ошибка при пересчете статистики маклеров.\n");
  }
}

// Task 5
void update_goods_quantity_and_clear_deals() {
  char date[11];
  safe_scanf("Введите дату (YYYY-MM-DD), до которой будут учтены сделки: ",
             date, sizeof(date));
  // TODO: Validate date format

  printf("Обновление остатков товаров и удаление сделок до %s...\n", date);

  execute_non_query("BEGIN TRANSACTION;");

  // Update Goods quantity based on deals up to the specified date
  // Using a subquery. Ensure Goods table exists and has data.
  // This query updates ALL goods based on deals before the date.
  char update_query[1024];
  snprintf(update_query, sizeof(update_query),
           "UPDATE Goods SET quantity = quantity - IFNULL(("
           "  SELECT SUM(d.sell_quantity) FROM Deals d "
           "  WHERE d.good_name_fk = Goods.name "
           "  AND d.supplier_name_fk = Goods.supplier_name_fk "
           "  AND d.deal_date <= '%s'"
           "), 0) "
           "WHERE EXISTS (" // Only update goods that actually had sales in the
                            // period
           "  SELECT 1 FROM Deals d "
           "  WHERE d.good_name_fk = Goods.name "
           "  AND d.supplier_name_fk = Goods.supplier_name_fk "
           "  AND d.deal_date <= '%s'"
           ");",
           date, date); // SQL Injection Risk

  int rc_update = execute_non_query(update_query);
  if (rc_update != SQLITE_OK) {
    execute_non_query("ROLLBACK;");
    printf("Ошибка при обновлении остатков товаров.\n");
    return;
  }
  printf("%d записей товаров обновлено (уменьшено количество).\n",
         sqlite3_changes(db));

  // Delete deals up to the specified date
  char delete_query[256];
  snprintf(delete_query, sizeof(delete_query),
           "DELETE FROM Deals WHERE deal_date <= '%s';",
           date); // SQL Injection Risk

  int rc_delete = execute_non_query(delete_query);
  if (rc_delete != SQLITE_OK) {
    execute_non_query("ROLLBACK;");
    printf("Ошибка при удалении сделок.\n");
    return;
  }
  printf("%d записей сделок удалено.\n", sqlite3_changes(db));

  // Commit the transaction if both operations were successful
  execute_non_query("COMMIT;");
  printf("Обновление остатков и очистка сделок до %s завершены.\n", date);
}

// Task 6
void show_deals_on_date() {
  char date[11];
  safe_scanf("Введите дату (YYYY-MM-DD) для просмотра сделок: ", date,
             sizeof(date));
  // TODO: Validate date format

  char query[512];
  snprintf(query, sizeof(query), "SELECT * FROM Deals WHERE deal_date = '%s';",
           date); // SQL Injection Risk
  execute_select_query(query);
}

// --- Broker Specific Function ---
// Added for broker role functionality
void show_broker_deals(const char *broker_surname) {
  if (!broker_surname) {
    printf("Ошибка: Фамилия маклера не указана.\n");
    return;
  }
  printf("\n--- Сделки для маклера: %s ---\n", broker_surname);
  char query[512];
  snprintf(query, sizeof(query),
           "SELECT deal_id, deal_date, good_name_fk, supplier_name_fk, "
           "type_of_good, sell_quantity, buyer_name_fk "
           "FROM Deals WHERE broker_surname_fk = '%s' ORDER BY deal_date DESC;",
           broker_surname); // SQL Injection Risk
  execute_select_query(query);
}