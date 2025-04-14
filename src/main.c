#include "db.h"
#include "queries.h"
#include <stdio.h>

int main() {
  if (open_db("../docs/ParfumeMarket.db") != 0)
    return 1;

  int choice;
  do {
    printf("\n=== Парфюмерный базар ===\n");
    printf("1. Продажи за период\n");
    printf("2. Покупатели по товару\n");
    printf("3. Самый популярный тип\n");
    printf("4. Маклер с макс. сделок\n");
    printf("5. Маклеры по поставщикам\n");
    printf("6. Обновить статистику\n");
    printf("7. Обновить остатки и очистить сделки\n");
    printf("8. Сделки на дату\n");
    printf("0. Выход\n");
    printf("Ваш выбор: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1:
      run_sales_summary_by_period();
      break;
    case 2:
      run_buyers_by_good();
      break;
    case 3:
      run_most_popular_type_info();
      break;
    case 4:
      run_top_broker_info();
      break;
    case 5:
      run_supplier_brokers_info();
      break;
    case 6:
      update_broker_stats();
      break;
    case 7:
      update_goods_quantity_and_clear_deals();
      break;
    case 8:
      show_deals_on_date();
      break;
    case 0:
      break;
    default:
      printf("Неверный пункт!\n");
    }
  } while (choice != 0);

  close_db();
  return 0;
}