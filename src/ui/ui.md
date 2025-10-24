# GTK LMS Application

## Main prompt

You are an expert UI developer in C programming language. You excel at creating responsive, interactive and appealing UI applications using the gtk framework in C language. Create a gtk application in C language (you may use gtk3 or gtk4 depending on which one you can work on better). It will be a basic Learning Management System which generates MCQs based on user specified topics. Alongside MCQ generation it also provides a student profile analyzer page.

## App details

The application would contain multiple pages including:

- Home page
- Sign in page
- Quiz page
- Profile analyzer page

### Home Page

Start with a student struct which stores all the useful information about the student including student_id, password, quiz_score, etc. A page where multiple redirection buttons will be shown to the user including mcq generator page, profile analyzer page which will redirect to the respective UI interfaces.

### Sign In Page

This page would contain two input fields for, one for student id in format: "25K-0000", another field for password. A button for submit which will check into an array of student structs to find if the entered student id and password exists. If it exists then add the matched student struct into the current student struct, if it does not exist then add the student id and password to the array and then also add into the current student struct.

### Quiz/MCQ Page

A dummy array will be made which will contain multiple placeholder mcqs with options combined with the question and the correct answer. The program will go through the array and will display each mcq one by one, then take the answer as input from the user then check into the array for correct answer to see if user's input is correct. If it is correct then display a notification saying "You answered correct" , do the same for incorrect answer then proceed to the next question.

### Student Profile Analyzer

A page with a form for entering student's personal data(Name, student_id, Current CGPA (overall and also for 9 individual courses), performance in attempted quizzes). Below it will showcase a scorecard of the current student's previous quizzes, performance metrics and their progress in their registered courses (use dummy data).
When the user fills in the form above and submits, show a hardcoded feedback/analysis of the profile info, to the user.
