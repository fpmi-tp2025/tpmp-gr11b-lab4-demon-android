#ifndef AUTH_H
#define AUTH_H

#include <stddef.h> // For size_t

#define MAX_USERNAME_LEN 50
#define MAX_ROLE_LEN 10
#define MAX_BROKER_SURNAME_LEN 100
#define MAX_PASSWORD_LEN 100 // Макс. длина пароля

// Structure to hold logged-in user info
typedef struct {
  char username[MAX_USERNAME_LEN];
  char role[MAX_ROLE_LEN];                     // "admin" or "broker"
  char broker_surname[MAX_BROKER_SURNAME_LEN]; // Filled if role is "broker"
  int is_authenticated;
  // Временное поле для хранения введенного пароля для проверки
  char current_password_attempt[MAX_PASSWORD_LEN];
} UserSession;

/**
 * @brief Attempts to authenticate a user.
 * Uses placeholder hashing/verification.
 * @param username Input username.
 * @param password Input password (plaintext).
 * @param session Pointer to UserSession structure to fill on success.
 * @return 0 on successful login, 1 on failure (bad credentials), -1 on error.
 */
int login_user(const char *username, const char *password,
               UserSession *session);

/**
 * @brief Placeholder for password hashing (replace with actual hashing).
 */
void hash_password(const char *password, char *hashed_output,
                   size_t output_size);

/**
 * @brief Placeholder for password verification (replace with actual hash
 * comparison). Compares hash(password) with hash_from_db.
 * @param password The plaintext password attempt.
 * @param hash_from_db The hash stored in the database.
 * @return 1 if match, 0 otherwise.
 */
int verify_password(const char *password, const char *hash_from_db);

#endif // AUTH_H