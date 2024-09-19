# Recipe For A Disaster
Recipe for a Disaster is a small recipe manager application, fundamentally built with C++ language and object-oriented programming. The goal is to build a functionable application that reads and stores recipes from a file passed. Currently attempting to implement JSON logic. Ideally there should exist functionality to enter a recipe via a website. This should help to improve concepts such as data management, file handling, and user interaction.

## Learning Outcomes
-	Develop knowledge of object-oriented programming through class design.
-	Improve your understanding of file handling and data persistence.
-	Learn to work with collections (like vectors) and algorithms for searching and sorting.
-	Enhance your skills in console application development, including user input validation and error handling.

## Design Considerations
1.	Classes and Data Structures:
	   - Recipe Class: Encapsulates properties like name, ingredients (could be a vector or list), instructions, cooking time, and category.
	   - RecipeManager Class: Handles all logic related to recipe management (adding, deleting, searching).
	   - User Interface Class: Manages console input and output, displaying menus and prompts.
2.	File I/O:
	    - Implement functions for reading from and writing to files for saving recipes in a structured format (like JSON or CSV).
        ** Changed this to mongoDB since drafting.
3.	Error Handling:
	    - Include validation for user inputs (e.g., ensuring ingredient lists are not empty).

## Next Steps
- File I/O Implementation: Implement the loadRecipes and saveRecipes methods in RecipeManager to read from and write to files.
- User Input Validation: Add error checking and validation for user inputs.
- Additional Features: Consider implementing search and filtering functionality, favorites, or a graphical user interface as you progress.
- Data Storage: JSON to start and need to look at a backend DB. 
•	Consider using nlohmann/json for easy JSON manipulation if you choose that format for data storage.
•	Changed this since writing to MongoDB.
