#include "../includes/auth.h" // Correct path
#include "../includes/db.h"   // Correct path
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

// --- VERY INSECURE HASHING PLACEHOLDER ---
// Replace with bcrypt, Argon2, or similar library
void hash_password(const char *password, char *hashed_output,
                   size_t output_size) {
  // Example: Simple 'hashing' (DO NOT USE - just for demonstration)
  if (password && hashed_output && output_size > 0) {
    snprintf(hashed_output, output_size, "hashed_%s", password);
  } else if (hashed_output && output_size > 0) {
    hashed_output[0] = '\0'; // Handle null password case
  }
}

int verify_password(const char *password, const char *hash_from_db) {
  if (!password || !hash_from_db) {
    printf("DEBUG: verify_password: Received NULL password or hash_from_db.\n");
    return 0; // Cannot verify if input is NULL
  }
  char temp_hash[256]; // Ensure buffer is large enough
  hash_password(password, temp_hash,
                sizeof(temp_hash)); // Hash the input password
  printf("DEBUG: verify_password: Comparing hash_from_input ('%s') with "
         "hash_from_db ('%s')\n",
         temp_hash, hash_from_db);
  int result = strcmp(temp_hash,
                      hash_from_db); // Compare generated hash with stored hash
  printf("DEBUG: verify_password: strcmp result = %d (0 means match)\n",
         result);
  return (
      result ==
      0); // Return 1 (true) if strcmp returns 0 (match), otherwise 0 (false)
}
// --- END INSECURE PLACEHOLDER ---

// Callback for login query
static int login_callback(void *data, int argc, char **argv, char **azColName) {
  UserSession *session = (UserSession *)data; // Get pointer to session struct
  const char *db_password_hash = NULL;
  const char *db_role = NULL;
  const char *db_broker_surname = NULL;

  printf("DEBUG: login_callback: Entered.\n");

  // Extract data from the row returned by SQLite
  for (int i = 0; i < argc; i++) {
    if (strcmp(azColName[i], "password_hash") == 0) {
      db_password_hash = argv[i];
    } else if (strcmp(azColName[i], "role") == 0) {
      db_role = argv[i];
    } else if (strcmp(azColName[i], "broker_surname_fk") == 0) {
      db_broker_surname = argv[i];
    }
  }

  // Basic validation
  if (!db_password_hash || !db_role) {
    fprintf(stderr, "!!! login_callback: Login query failed to retrieve "
                    "necessary user data (hash or role is NULL).\n");
    session->is_authenticated = 0; // Ensure flag is reset
    return 1; // Indicate error (stop processing if possible)
  }

  printf("DEBUG: login_callback: Found user '%s', role '%s'. Verifying "
         "password...\n",
         session->username, db_role);

  // --- !!! CORRECTED call to verify_password !!! ---
  // Use the password temporarily stored in the session structure
  if (verify_password(session->current_password_attempt, db_password_hash)) {
    // Password matches
    printf("DEBUG: login_callback: Password verified successfully.\n");
    session->is_authenticated = 1; // Set authentication flag
    strncpy(session->role, db_role, sizeof(session->role) - 1);
    session->role[sizeof(session->role) - 1] = '\0'; // Ensure null termination

    // Handle broker-specific info
    if (strcmp(session->role, "broker") == 0 && db_broker_surname) {
      strncpy(session->broker_surname, db_broker_surname,
              sizeof(session->broker_surname) - 1);
      session->broker_surname[sizeof(session->broker_surname) - 1] = '\0';
      printf("DEBUG: login_callback: Broker surname set to '%s'.\n",
             session->broker_surname);
    } else {
      session->broker_surname[0] =
          '\0'; // Clear if not a broker or no surname linked
    }
    return 0; // Success - stop sqlite3_exec from processing more rows (though
              // LIMIT 1 should prevent it)
  } else {
    // Password mismatch
    printf("DEBUG: login_callback: Password verification failed.\n");
    session->is_authenticated = 0; // Ensure flag is reset
    return 1; // Indicate failure - stop sqlite3_exec from processing more rows
  }
}

// Login function
int login_user(const char *username, const char *password,
               UserSession *session) {
  if (!db) {
    fprintf(stderr, "!!! Database not open for login.\n");
    return -1; // DB error
  }
  if (!username || !password || !session) {
    fprintf(stderr,
            "!!! login_user: Invalid arguments (NULL pointer passed).\n");
    return -1; // Programming error
  }

  printf("DEBUG: login_user: Attempting login for user '%s'\n", username);

  // Initialize session struct (clear previous state)
  memset(session, 0, sizeof(UserSession));
  strncpy(session->username, username, sizeof(session->username) - 1);
  session->username[sizeof(session->username) - 1] =
      '\0'; // Ensure null termination

  // --- !!! Temporarily store password in session !!! ---
  // INSECURE for production - password should be hashed ASAP
  strncpy(session->current_password_attempt, password,
          sizeof(session->current_password_attempt) - 1);
  session->current_password_attempt[sizeof(session->current_password_attempt) -
                                    1] = '\0';
  // --- End temp password storage ---

  char query[512];
  // !!! VULNERABLE TO SQL INJECTION - Use prepared statements !!!
  snprintf(query, sizeof(query),
           "SELECT password_hash, role, broker_surname_fk FROM Users WHERE "
           "username = '%s' LIMIT 1;",
           username);

  char *errMsg = NULL;
  printf("DEBUG: login_user: Executing query: %s\n", query);

  // Execute the query, passing the session struct to the callback
  int rc = sqlite3_exec(db, query, login_callback, session, &errMsg);

  // --- !!! Clear password from memory ASAP !!! ---
  // Overwrite the temporary storage
  memset(session->current_password_attempt, 0,
         sizeof(session->current_password_attempt));
  // --- End password clearing ---

  // Check execution result
  if (rc != SQLITE_OK && rc != SQLITE_ABORT) {
    // SQLITE_ABORT means the callback requested stop (which it does on
    // success/failure)
    fprintf(stderr, "!!! SQL error during login query execution: %s (rc=%d)\n",
            errMsg ? errMsg : "Unknown error", rc);
    sqlite3_free(errMsg);
    session->is_authenticated = 0; // Ensure not authenticated on error
    return -1;                     // Database error
  }

  // Check the authentication flag set by the callback
  if (!session->is_authenticated) {
    // Failure message printed by callback or main loop
    return 1; // Authentication failed (user not found or password mismatch)
  }

  // If we reach here, session->is_authenticated must be 1
  printf("DEBUG: login_user: Login successful for user '%s'. Role: '%s'.\n",
         session->username, session->role);
  if (strcmp(session->role, "broker") == 0) {
    printf("DEBUG: login_user: Broker surname is '%s'.\n",
           session->broker_surname);
  }
  return 0; // Login successful
}