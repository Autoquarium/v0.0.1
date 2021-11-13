
// this function has all the CLI logic for users to setup their new product
// to use: make a Menu object and then call the member function loop()


#include <Preferences.h>
#include <WiFi.h>


class Menu {

	// Variables

	// Commands
	const String version = "0.0.1";
	const String get_input = "GETIN"; // get input from the user
	const String get_pass = "GETPS"; // get password input from the user
	const String clear_screen = "CLRSCR"; // clear the entire screen
	const String exit_cli = "EXIT"; // exit the CLI tool

	// value of last user menu selection
	volatile int selection = 0;

	// function pointer to the current menu
	int (Menu::*current_menu)();


	Preferences preferences;


	// FUNCTIONS

	/**
	 * @brief sends start message to the application on user PC, timout after 12 seconds
	 * 
	 * @return int 1: connected to application on computer
	 * 			   2: did not connect to application
	 */
	int send_start() {
	  unsigned long start = millis();
	  while (millis() - start < 12000) {
	  	Serial.println("blub-blub-blub");
	    if (Serial.available()) {
		    String x = Serial.readString();
		    if (x == "hello-fish") {
		      return 1;
		    }
		}
	  }
	  return 0;
	}


	/**
	 * @brief Get the Input from the user typing on the PC application
	 * 
	 * @return String the user input
	 */
	String getInput() {
	    String ret;
	    Serial.println(get_input);
	    while(!Serial.available());
	    ret += Serial.readString();// read the incoming data as string
	    return ret;
	}


	/**
	 * @brief Get the Password that the user is typing on the PC application
	 * 
	 * @return String the user entered password
	 */

	String getPassword() {
	    String ret;
	    Serial.println(get_pass);
	    while(!Serial.available());
	    ret += Serial.readString();// read the incoming data as string
	    return ret;
	}


	/**
	 * @brief Asks the user to make a section of options (1,2,3,ect..)
	 * 
	 * @return int the selection made by the user
	 */
	int makeSelection() {
	  return getInput().toInt();
	}


	/**
	 * @brief Admin tool that allows for admin to change local variables and setting on the device
	 * 
	 */
	void adminFunction() {
	  // TODO:
	  // 0) Back // back to main menu
	  // 1) Print all
	  // 2) Edit
	      // enter var_name
	      // enter new_value
	}


	/**
	 * @brief Tests the connection for a given wifi 
	 * 
	 * @param ssid_in the name of the wifi network to test
	 * @param passwrd_in password to the wifi network to connet to
	 */
	void testWifi(String ssid_in, String passwrd_in) {

	  char ssid[30];
	  char passwrd[30];

	  ssid_in.toCharArray(ssid, ssid_in.length() + 1);
	  passwrd_in.toCharArray(passwrd, passwrd_in.length() + 1);


	  int status = WL_IDLE_STATUS;
	  Serial.println("Testing Connection . . .");
	  
	  WiFi.begin(ssid, passwrd);
	  
	  unsigned long start = millis();
	  while (status != WL_CONNECTED) {
	    status = WiFi.begin(ssid, passwrd);
	    delay(8000);
	    if (millis() - start > 40000) {
	    	Serial.println("Connection Failed");
	    	return;
	    }
	  }

	  Serial.println("Connection Successful!");
	  Serial.println("Connection Strength: " + String(WiFi.RSSI()));
	}



	// MENU DRAW FUNCTIONS
	// all the functions below are for drawing menus


	/**
	 * @brief draws the admin menu
	 * 
	 * @return int the number associated with the admin menu
	 */
	int adminMenu() {
	  Serial.println(clear_screen);
	  Serial.println("Admin Menu\n");
	  
	  selection = makeSelection(); // TODO: replace with adminFunction();
	  current_menu = &Menu::menu1;
	  selection = -1;
	  return 6;
	}


	/**
	 * @brief draws the admin login menu, prompts user for input
	 * 
	 * @return int the number associated with the admin login menu
	 */
	int adminLogin() {
	  Serial.println(clear_screen);
	  Serial.println("Admin Login\n");
	  Serial.println("Enter Admin Password: ");
	  String password = getPassword();

	  if (password != "test") {
	    Serial.println("\n\nPassword Incorrect!");
	    Serial.println("---------------");
	    Serial.println("  0) Back"); // go to main menu
	    Serial.println("  1) Retry"); // just clear and keep going
	    selection = makeSelection();
	  }
	  else {
	    current_menu = &Menu::adminMenu;
	    selection = -1;
	  }
	  return 5;
	}


	/**
	 * @brief draws the device info menu
	 * 
	 * @return int the number associated with the device info menu
	 */
	int deviceInfoMenu() {
	  Serial.println(clear_screen);
	  Serial.println("Device Information:");
	  Serial.println("    Serial Number: 1");
	  Serial.println("    Dashboard URL: https://autoquarium.app/dashboard/d/8ecf97a7-5e0c-41dc-bd1a-770f5e9f716a");
	  Serial.println("");

	  // get values for printing
	  preferences.begin("saved-values", false);
	  String ssid = preferences.getString("wifi_SSID", "no value"); 
	  String password = preferences.getString("wifi_PWD", "no value");
	  String alert_usr = preferences.getString("alert_usr", "no value");
	  int time_off = preferences.getInt("time_zone", -5);
	  preferences.end();

	  Serial.println("Other Information:");
	  Serial.println("    WiFi SSD: " + ssid);
	  Serial.println("    WiFi Password: " + password);
	  Serial.println("    PushOver User Key: " + alert_usr);
	  Serial.println("    Timezone offset: " + String(time_off));
	  Serial.println("");
	  Serial.println("----------------------");
	  Serial.println("  0) Back");
	  selection = makeSelection();
	  return 4;
	}


	/**
	 * @brief draws the timezone menu and propts for user input
	 * 
	 * @return int the number associated with the time zone menu
	 */
	int timeZoneSetup() {
	  Serial.println(clear_screen);
	  Serial.println("Setup TimeZone\n");
	  Serial.println("input number and press enter:");
	  Serial.println("  0) EST (GMT-5:00)");
	  Serial.println("  1) CST (GMT-6:00)");
	  Serial.println("  2) MST (GMT-7:00)");
	  Serial.println("  3) PST (GMT-8:00)");
	  int timezone = (makeSelection() + 5) * -1;

	  // save timezone to non-vol memory
	  preferences.begin("saved-values", false);
	  preferences.putInt("time_zone", timezone); 
	  preferences.end();
	  return 3;
	}


	/**
	 * @brief draws the menu for notification setup, prompts user for input
	 * 
	 * @return int the number associated with the notifcation setup menu 
	 */
	int notiSetup() {
	  Serial.println(clear_screen);
	  Serial.println("Enter PushOver User Key: ");
	  String alert_usr = getInput();
	  
	  preferences.begin("saved-values", false);
	  preferences.putString("alert_usr", alert_usr); 
	  preferences.end();
	  return 2;
	}


	/**
	 * @brief draws the menu for wifi setup, prompts user for inputs
	 * 
	 * @return int the number associated with the wifi setup menu
	 */
	int wifiMenu() {
	  Serial.println(clear_screen);
	  Serial.println("Enter WiFi SSID: ");
	  String wifi_SSID = getInput();
	  Serial.println("Enter WiFi Pasword: ");
	  String wifi_PWD = getInput();
	  testWifi(wifi_SSID, wifi_PWD);
	  delay(1000);

	  Serial.println("\n\n---------------");
	  Serial.println("  0) Back");
	  Serial.println("  1) Re-enter"); // reload wifiMenu
	  Serial.println("  2) Save"); // save the vars locally
	  selection = makeSelection();

	  if (selection == 2) {
	  	preferences.begin("saved-values", false);
	  	preferences.putString("wifi_SSID", wifi_SSID); 
	  	preferences.putString("wifi_PWD", wifi_PWD);
	  	preferences.end();
	  }

	  return 1;
	}


	/**
	 * @brief draws the main menu, prompts user for input
	 *  
	 * @return int the number associated with the main menu 
	 */
	int menu1() {
	  Serial.println(clear_screen);
	  Serial.println("input number and press enter:");
	  Serial.println("  0) Quit");
	  Serial.println("  1) WiFi setup");
	  Serial.println("  2) Notifcation setup");
	  Serial.println("  3) Timezone setup");
	  Serial.println("  4) Device info");
	  Serial.println("  5) Admin tools");
	  Serial.println("");
	  selection = makeSelection();
	  return 0;
	}


	/**
	 * @brief determines the next menu given the current menu and the user selection
	 * NOTE: this function triggers a reboot of the processor when the menu is quited
	 * 
	 * @param menu_val the number associated with the current menu
	 */
	void execute(int menu_val) {
	  
	  if (selection == -1) {
	    return;
	  }
	  
	  // main menu
	  if (menu_val == 0) {
	    if (selection == 0) {
	      Serial.println(clear_screen);
	      Serial.println("Goodbye!");
	      Serial.println(exit_cli);
	      ESP.restart(); // start the entire processor
	    }
	    else if (selection == 1) {
	      current_menu = &Menu::wifiMenu;
	    } else if (selection == 2) {
	      current_menu = &Menu::notiSetup;
	    } else if (selection == 3) {
	      current_menu = &Menu::timeZoneSetup;
	    } else if (selection  == 4) {
	      current_menu = &Menu::deviceInfoMenu;
	    } else if (selection  == 5) {
	      current_menu = &Menu::adminLogin;
	    }
	  } else if (menu_val == 1) {
	    if (selection == 0 || selection == 2) {
	      current_menu = &Menu::menu1;
	    }
	  } else if (menu_val == 2) {
	    current_menu = &Menu::menu1;
	  } else if(menu_val == 3) {
	    current_menu = &Menu::menu1;
	  } else if(menu_val == 4) {
	    current_menu = &Menu::menu1;
	  }
	}

public:

	/**
	 * @brief loops through all the menus as the user inputs values
	 * 
	 */
	void loop() {
		if (send_start()) {
			current_menu = &Menu::menu1;
			while(1) {
				int menu_val = (this->*current_menu)(); //Calling a member function from another member function using pointer to member
	  			execute(menu_val);
			}
		}
	}
};