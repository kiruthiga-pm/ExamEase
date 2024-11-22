#include <iostream>
#include <vector>
#include <map>
#include <mysql.h>
#include <sstream>
#include <ctime>
#include <string>
#include <algorithm> // For std::shuffle
#include <random>    // For std::default_random_engine
#include <chrono>    // For seeding the random engine
using namespace std;
class Database {
public:
    MYSQL* conn;
    Database() {
        conn = mysql_init(0);
        if (conn) {
            string server = "localhost";
            string user = "root";
            string password = "PMkiruthi@2004"; // MySQL password
            string database = "examease";
            conn = mysql_real_connect(conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 3306, NULL, 0);
        }
        if (!conn) {
            cerr << "Connection Error: " << mysql_error(conn) << endl;
        }
    }
    ~Database() {
        if (conn) {
            mysql_close(conn);
        }
    }
    bool executeQuery(const string& query) {
        return mysql_query(conn, query.c_str()) == 0;
    }
    MYSQL_RES* storeResult() {
        return mysql_store_result(conn);
    }
};

class User {
protected:
    string username;
    string password;
public:
    User(string uname, string pword) : username(uname), password(pword) {}
    virtual bool validateCredentials(Database& db,string role_name) {
        string query = "SELECT COUNT(*) FROM login WHERE name = '" + username + "' AND password = '" + password + "' AND role = '" + role_name + "';";
        if (db.executeQuery(query)) {
            MYSQL_RES* res = db.storeResult();
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row && atoi(row[0]) > 0) {
                cout << "Login successful!" << endl;
                mysql_free_result(res);
                return true;
            } else {
                cout << "Invalid username or password." << endl;
                mysql_free_result(res);
                return false;
            }
        }
        return false;
    }
    virtual void login() = 0;
};
class Question {
private:
    int questionid;
    string questionText;
    vector<string> options;
    int correctOption;
    int marks;

public:
    Question() : questionid(0), correctOption(0), marks(0) {}
    Question(int id, string qText, vector<string> opts, int correctOpt, int m)
        : questionid(id), questionText(qText), options(opts), correctOption(correctOpt), marks(m) {}

    void displayQuestion() const {
    	cout << "Question id: " << questionid << endl;
        cout << "Question: " << questionText << endl;
        for (int i = 0; i < options.size(); i++) {
            cout << i + 1 << ". " << options[i] << endl;
        }
        cout << "Marks: " << marks << endl;
    }

    bool checkAnswer(int studentChoice) const {
        return studentChoice == correctOption;
    }

    void setQuestionText(const string& qText) {
        questionText = qText;
    }
    void setOptions(const vector<string>& opts) {
        options = opts;
    }
    void setCorrectOption(int correctOpt) {
        correctOption = correctOpt;
    }
    void setMarks(int m) {
        marks = m;
    }

    int getQuestionId() const {
        return questionid;
    }
    string getQuestionText() const {
        return questionText;
    }
    vector<string> getOptions() const {
        return options;
    }
    int getCorrectOption() const {
        return correctOption;
    }
    int getMarks() const {
        return marks;
    }
    void setQuestionId(int id) {
        questionid = id;
    }
    int getId(){
    	return questionid;
	}
};
class Exam {
private:
    int examId;
    time_t startTime;
    time_t endTime;
    vector<Question> questions;
    int duration;

public:
    Exam() : examId(0), duration(0), startTime(0), endTime(0) {}
    Exam(int id, time_t start, time_t end, int dur)
        : examId(id), startTime(start), endTime(end), duration(dur) {
        Database database;
		string query = "SELECT question_id, question_text, option1, option2, option3, option4, "
                   "correct_option, marks FROM questions WHERE exam_id = " + to_string(examId);

    if (database.executeQuery(query)) {
        MYSQL_RES* result = database.storeResult();
        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                try {
                    int questionId = stoi(row[0]);
                    string questionText = row[1];
                    vector<string> options = { row[2], row[3], row[4], row[5] };
                    int correctOption = stoi(row[6]);
                    int marks = stoi(row[7]);
                    Question question(questionId, questionText, options, correctOption, marks);
                    questions.push_back(question);
                } catch (const invalid_argument& e) {
                    cerr << "Invalid argument: " << e.what() << endl;
                } catch (const out_of_range& e) {
                    cerr << "Out of range: " << e.what() << endl;
                }
            }
            mysql_free_result(result);
            cout << "Questions loaded for Exam ID: " << examId << " successfully." << endl;
        } else {
            cerr << "Error fetching results: " << mysql_error(database.conn) << endl;
        }
    } else {
        cerr << "Error executing query: " << mysql_error(database.conn) << endl;
    }
	}

    void addQuestion(const Question& q) {
        questions.push_back(q);
    }

    bool deleteQuestion(int questionId) {
    for (auto it = questions.begin(); it != questions.end(); ++it) {
        if (it->getId() == questionId) {
            questions.erase(it);
            cout << "Question deleted successfully." << endl;
            return true;
        }
    }
    cout << "Question ID not found!" << endl;
    return false;
}

    void updateQuestion(int questionId, const Question& updatedQuestion) {
        for (auto& question : questions) {
            if (question.getId() == questionId) {
                question = updatedQuestion;
                cout << "Question updated successfully." << endl;
                return;
            }
        }
        cout << "Question ID not found!" << endl;
    }

    void displayExam() const {
        cout << "Exam ID: " << examId << endl;
        cout << "Starts at: " << ctime(&startTime);
        cout << "Ends at: " << ctime(&endTime) << endl;
        cout << "Duration: " << duration << " minutes" << endl;
        cout << "Questions:" << endl;
        for (int i = 0; i < questions.size(); i++) {
            cout << "Q" << i + 1 << ": ";
            questions[i].displayQuestion();
            cout << endl;
        }
    }

    bool isExamActive() const {
        time_t now = time(0);
        return (now >= startTime && now <= endTime);
    }

    vector<Question>& getQuestions() {
        return questions;
    }

    const vector<Question>& getQuestions() const {
        return questions;
    }

    int getExamId() const {
        return examId;
    }

    void setStartTime(time_t newStart) {
        startTime = newStart;
    }

    void setEndTime(time_t newEnd) {
        endTime = newEnd;
    }

    time_t getStartTime() const {
        return startTime;
    }

    time_t getEndTime() const {
        return endTime;
    }

    int getDuration() const {
        return duration;
    }

    void setDuration(int dur) {
        duration = dur;
    }
};
class Result {
private:
    int totalScore;
    map<int, map<int, int>> questionScores; // examId -> (questionId -> marks obtained)

public:
    Result() : totalScore(0) {}
    Result(int s) : totalScore(s) {}

    void addScore(int examId, int questionId, int marksObtained) {
        questionScores[examId][questionId] = marksObtained;
        totalScore += marksObtained;
    }

    void displayResult() const {
        cout << "Total Score: " << totalScore << endl;
       for (const auto& exam : questionScores) {
    int examId = exam.first;
    cout << "Exam ID " << examId << ":" << endl;

    for (const auto& question : exam.second) {
        int questionId = question.first;
        int marks = question.second;
        cout << "  Question " << questionId << ": " << marks << " marks" << endl;
    }
}

    }
};

class Faculty : public User {
private:
    map<int, Exam> exams; // Store exams created by the faculty
    Database database;
public:
    Faculty(string uname, string pword) : User(uname, pword) {
	string query = "SELECT exam_id, UNIX_TIMESTAMP(start_time) AS start_time, "
                       "UNIX_TIMESTAMP(end_time) AS end_time, duration FROM exams";

        if (database.executeQuery(query)) {
            MYSQL_RES* result = database.storeResult();
            if (result) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result))) {
                    try {
                        int examId = stoi(row[0]);
                        time_t startTime = static_cast<time_t>(stoi(row[1]));
                        time_t endTime = static_cast<time_t>(stoi(row[2]));
                        int duration = stoi(row[3]);
                        exams[examId] = Exam(examId, startTime, endTime, duration);
                    } catch (const invalid_argument& e) {
                        cerr << "Invalid argument: " << e.what() << endl;
                    } catch (const out_of_range& e) {
                        cerr << "Out of range: " << e.what() << endl;
                    }
                }
                mysql_free_result(result); // Free the result set
                cout << "Exams loaded from database to objects successfully." << endl;
            } else {
                cerr << "Error fetching results: " << mysql_error(database.conn) << endl;
            }
        } else {
            cerr << "Error executing query: " << mysql_error(database.conn) << endl;
        }
	}
    void login() override {
        cout << "Faculty login successful!" << endl;
    }
    void createExam(int examId);
    void deleteExam(int examId);
    void randomizeQuestionOrder(int examId);
    void addQuestionToExam(int examId, string questionText, vector<string> options, int correctOption, int marks);
    void updateQuestionInExam(int examId, int questionIndex, string newQuestionText, vector<string> newOptions, int newCorrectOption, int newMarks);
    void deleteQuestionFromExam(int examId, int questionIndex);
    void setExamTimings(int examId, time_t newStart, time_t newEnd);
    void updateExamTimings(int examId, time_t newStart, time_t newEnd);
    void showExam(int examId);
    Exam& getExam(int examId) {
        return exams[examId];
    }
    bool examExists(int examId) const {
        return exams.find(examId) != exams.end();
    }
};
class Student : public User {
private:
    map<int, Result> results;
    map<int, map<int, int>> answers;

public:
    Student(string uname, string pword) : User(uname, pword) {
        Database database;
        string query = "SELECT exam_id, score FROM results WHERE username = '" + uname + "';";

        if (database.executeQuery(query)) {
            MYSQL_RES* result = database.storeResult();
            if (result) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result))) {
                    try {
                        int examId = stoi(row[0]);
                        int score = stoi(row[1]);
                        results[examId] = Result(score);
                    } catch (const invalid_argument& e) {
                        cerr << "Invalid argument: " << e.what() << endl;
                    } catch (const out_of_range& e) {
                        cerr << "Out of range: " << e.what() << endl;
                    }
                }
                mysql_free_result(result);
                cout << "Results loaded from database to objects successfully." << endl;
            } else {
                cerr << "Error fetching results: " << mysql_error(database.conn) << endl;
            }
        } else {
            cerr << "Error executing query: " << mysql_error(database.conn) << endl;
        }

        query = "SELECT exam_id, question_id, answer FROM answers WHERE username = '" + uname + "';";
        if (database.executeQuery(query)) {
            MYSQL_RES* result = database.storeResult();
            if (result) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result))) {
                    try {
                        int examId = stoi(row[0]);
                        int questionId = stoi(row[1]);
                        int answer = stoi(row[2]);
                        answers[examId][questionId] = answer;
                    } catch (const invalid_argument& e) {
                        cerr << "Invalid argument: " << e.what() << endl;
                    } catch (const out_of_range& e) {
                        cerr << "Out of range: " << e.what() << endl;
                    }
                }
                mysql_free_result(result);
                cout << "Answers loaded from database to objects successfully." << endl;
            } else {
                cerr << "Error fetching answers: " << mysql_error(database.conn) << endl;
            }
        } else {
            cerr << "Error executing query: " << mysql_error(database.conn) << endl;
        }
    }

    void login() override {
        cout << "Student login successful!" << endl;
    }

    void takeExam(Exam &exam, const string &username);
    void viewResult(int examId) const;
    void updateAnswer(int examId, int questionId, int newAnswer, Exam &exam);
};
void Faculty::createExam(int examId) {
    if (exams.find(examId) != exams.end()) {
        cout << "Exam with this ID already exists!" << endl;
        return;
    }
    time_t now = time(0);
    time_t end = now + 3600; // Default duration: 1 hour from now
    Exam newExam(examId, now, end, 60);
    exams[examId] = newExam;
    string query = "INSERT INTO exams (exam_id, start_time, end_time, duration) VALUES (" +to_string(examId) + ", FROM_UNIXTIME(" + to_string(now) +"), FROM_UNIXTIME(" + to_string(end) + "), " +to_string(newExam.getDuration()) + ")";
    if (database.executeQuery(query)) {
        cout << "Exam created with ID: " << examId << endl;
    } else {
        cerr << "Error creating exam in database: " << mysql_error(database.conn) << endl;
    }
}
void Faculty::deleteExam(int examId) {
    if (exams.erase(examId) > 0) {
        cout << "Exam deleted in the object.\n";

        string queryResults = "DELETE FROM results WHERE exam_id = " + to_string(examId) + ";";
        string queryAnswers = "DELETE FROM answers WHERE exam_id = " + to_string(examId) + ";";
        string queryExams = "DELETE FROM exams WHERE exam_id = " + to_string(examId) + ";";

        if (database.executeQuery(queryResults)) {
            cout << "Results for exam ID " << examId << " deleted successfully from the database." << endl;
        } else {
            cerr << "Failed to delete results for exam ID " << examId << " from the database." << endl;
        }

        if (database.executeQuery(queryAnswers)) {
            cout << "Answers for exam ID " << examId << " deleted successfully from the database." << endl;
        } else {
            cerr << "Failed to delete answers for exam ID " << examId << " from the database." << endl;
        }

        if (database.executeQuery(queryExams)) {
            cout << "Exam with ID " << examId << " deleted successfully from the database." << endl;
        } else {
            cerr << "Failed to delete exam with ID " << examId << " from the database." << endl;
        }
    } else {
        cout << "Exam with ID " << examId << " not found in local records!" << endl;
    }
}


void Faculty::addQuestionToExam(int examId, string questionText, vector<string> options, int correctOption, int marks) {
    if (exams.find(examId) != exams.end()) {
        if (options.size() < 1 || options.size() > 4) {
            cout << "You must provide between 1 and 4 options!" << endl;
            return;
        }
        string query = "INSERT INTO questions (exam_id, question_text, option1, option2, option3, option4, correct_option, marks) VALUES (" +
                       to_string(examId) + ", '" + questionText + "', '" +
                       (options.size() > 0 ? options[0] : "") + "', '" +
                       (options.size() > 1 ? options[1] : "") + "', '" +
                       (options.size() > 2 ? options[2] : "") + "', '" +
                       (options.size() > 3 ? options[3] : "") + "', " +
                       to_string(correctOption) + ", " +
                       to_string(marks) + ");";
        if (database.executeQuery(query)) {
            cout << "Question added to Exam ID: " << examId << endl;
            string fetchIdQuery = "SELECT question_id FROM questions WHERE exam_id = " + to_string(examId) +
                                   " AND question_text = '" + questionText + "' " +
                                   "AND option1 = '" + (options.size() > 0 ? options[0] : "") + "' " +
                                   "AND option2 = '" + (options.size() > 1 ? options[1] : "") + "' " +
                                   "AND option3 = '" + (options.size() > 2 ? options[2] : "") + "' " +
                                   "AND option4 = '" + (options.size() > 3 ? options[3] : "") + "' " ;

            if (database.executeQuery(fetchIdQuery)) {
                MYSQL_RES* res = database.storeResult();
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row) {
                    int questionId = atoi(row[0]);
                    Question q(questionId, questionText, options, correctOption, marks);
                    exams[examId].addQuestion(q);
                } else {
                    cout << "Failed to retrieve question ID!" << endl;
                }
                mysql_free_result(res);
            } else {
                cout << "Failed to execute query to retrieve question ID!" << endl;
            }
        } else {
            cout << "Failed to add question to the database!" << endl;
        }
    } else {
        cout << "Exam not found!" << endl;
    }
}
void Faculty::updateQuestionInExam(int examId, int questionIndex, string newQuestionText, vector<string> newOptions, int newCorrectOption, int newMarks) {
    if (exams.find(examId) != exams.end()) {
        Exam& exam = exams[examId];
        Question updatedQuestion(questionIndex,newQuestionText, newOptions, newCorrectOption, newMarks);
        exam.updateQuestion(questionIndex, updatedQuestion);
        string query = "UPDATE questions SET question_text = '" + newQuestionText + "', " +
                           "option1 = '" + (newOptions.size() > 0 ? newOptions[0] : "") + "', " +
                           "option2 = '" + (newOptions.size() > 1 ? newOptions[1] : "") + "', " +
                           "option3 = '" + (newOptions.size() > 2 ? newOptions[2] : "") + "', " +
                           "option4 = '" + (newOptions.size() > 3 ? newOptions[3] : "") + "', " +
                           "correct_option = " + to_string(newCorrectOption) + ", " +
                           "marks = " + to_string(newMarks) + " " +
                           "WHERE exam_id = " + to_string(examId) + " AND question_id = " + to_string(questionIndex)+ ";";
        if (database.executeQuery(query)) {
                cout << "Question text updated successfully in Exam ID: " << examId << endl;
        } else {
                cout << "Failed to update question text in the database!" << endl;
        }
        } else {
            cout << "Invalid question index!" << endl;
        }
}
void Faculty::deleteQuestionFromExam(int examId, int questionId) {
    if (exams.find(examId) != exams.end()) {
        if (exams[examId].deleteQuestion(questionId)) {
        	string query1 = "DELETE FROM answers WHERE exam_id = " + to_string(examId) + " AND question_id = " + to_string(questionId) + ";";
            string query = "DELETE FROM questions WHERE exam_id = " + to_string(examId) + " AND question_id = " + to_string(questionId) + ";";
            if (database.executeQuery(query1)) {
                cout << "Question deleted successfully from answers table in database." << endl;
            } else {
                cout << "Failed to delete question from answers table in database!" << endl;
            }
            if (database.executeQuery(query)) {
                cout << "Question deleted successfully from database." << endl;
            } else {
                cout << "Failed to delete question from database!" << endl;
            }
        } else {
            cout << "Question ID not found in exam!" << endl;
        }
    } else {
        cout << "Exam not found!" << endl;
    }
}
void Faculty::setExamTimings(int examId, time_t newStart, time_t newEnd) {
    if (exams.find(examId) != exams.end()) {
        Exam& exam = exams[examId];
        exam.setStartTime(newStart);
        exam.setEndTime(newEnd);
        string query = "UPDATE exams SET start_time = FROM_UNIXTIME(" + to_string(newStart) + 
                       "), end_time = FROM_UNIXTIME(" + to_string(newEnd) + 
                       ") WHERE exam_id = " + to_string(examId) + ";";
        if (database.executeQuery(query)) {
            cout << "Exam timings updated successfully in both system and database." << endl;
        } else {
            cout << "Failed to update exam timings in the database!" << endl;
        }
    } else {
        cout << "Exam not found!" << endl;
    }
}
void Faculty::updateExamTimings(int examId, time_t newStart, time_t newEnd) {
    setExamTimings(examId, newStart, newEnd);
}
void Faculty::showExam(int examId) {
    if (exams.find(examId) != exams.end()) {
        exams[examId].displayExam();
    } else {
        cout << "Exam not found!" << endl;
    }
}
void Faculty::randomizeQuestionOrder(int examId) {
    if (exams.find(examId) != exams.end()) {
        auto& questions = exams[examId].getQuestions(); // Get the questions vector
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(questions.begin(), questions.end(), default_random_engine(seed));
        cout << "Questions randomized successfully." << endl;
    } else {
        cout << "Exam not found!" << endl;
    }
}
void Student::takeExam(Exam &exam, const string &username) {
    Database database;

    if (!exam.isExamActive()) {
        cout << "Exam is not active!" << endl;
        return;
    }

    vector<Question>& questions = exam.getQuestions();
    int score = 0;

    for (size_t i = 0; i < questions.size(); i++) {
        questions[i].displayQuestion();
        int choice;
        cout << "Your answer (enter option number): ";
        cin >> choice;

        if (choice > 0 && choice <= static_cast<int>(questions[i].getOptions().size())) {
            int questionId = questions[i].getId();
            answers[exam.getExamId()][questionId] = choice;

            if (questions[i].checkAnswer(choice)) {
                score += questions[i].getMarks();
            }
        } else {
            cout << "Invalid choice! Skipping question." << endl;
        }
        cout << endl;
    }

    results[exam.getExamId()] = Result(score);

    string queryResult = "INSERT INTO results (username, exam_id, score) VALUES ('" +
                         username + "', " + to_string(exam.getExamId()) + ", " +
                         to_string(score) + ")";
    
    if (database.executeQuery(queryResult)) {
        cout << "Exam results saved. Your score: " << score << " marks." << endl;
    } else {
        cerr << "Failed to save exam results in the database!" << endl;
    }

    bool allAnswersSaved = true;
    for (const auto& exam1 : answers){
    	if(exam1.first==exam.getExamId()){
    	for(const auto& quest: exam1.second){
        string queryAnswer = "INSERT INTO answers (username, exam_id, question_id, answer) VALUES ('" +
                             username+ "', " + to_string(exam1.first) + ", " +
                             to_string(quest.first) + ", " + to_string(quest.second) + ");";
        if (!database.executeQuery(queryAnswer)) {
            cerr << "Failed to save answer for Question ID: " << quest.first<< endl;
            allAnswersSaved = false;
        }
    }
    }
    }

    if (allAnswersSaved) {
        cout << "All answers saved successfully." << endl;
    }
}
void Student::viewResult(int examId) const {
    auto it = results.find(examId);
    if (it != results.end()) {
        it->second.displayResult();
    } else {
        cout << "Result not found for exam ID: " << examId << endl;
    }
}
void Student::updateAnswer(int examId, int questionIndex, int newAnswer, Exam &exam) {
    Database database;
    auto it = answers.find(examId);
    if (it != answers.end()) {
        it->second[questionIndex] = newAnswer;

        string queryResult = "UPDATE answers SET answer = " + to_string(newAnswer) +
                             " WHERE username = '" + username + 
                             "' AND exam_id = " + to_string(examId) + 
                             " AND question_id = " + to_string(questionIndex) + ";";

        if (database.executeQuery(queryResult)) {
            cout << "Answers updated in database for question index " << questionIndex << endl;
        } else {
            cerr << "Failed to update answers in the database!" << endl;
        }

        int score = 0;
        vector<Question>& questions = exam.getQuestions();
        for (auto& ques : questions) {
            int questionId = ques.getId(); // Use the correct method to get the question ID
            if (it->second[questionId] == ques.getCorrectOption()) {
                score += ques.getMarks();
            }
        }

        results[examId] = Result(score);

        queryResult = "UPDATE results SET score = " + to_string(score) +
                      " WHERE username = '" + username + 
                      "' AND exam_id = " + to_string(examId) + ";";

        if (database.executeQuery(queryResult)) {
            cout << "Exam results updated in database. Your score: " << score << " marks." << endl;
        } else {
            cerr << "Failed to save exam results in the database!" << endl;
        }

        cout << "Answer updated successfully. New score: " << score << " marks." << endl;
    } else {
        cout << "No answers found for exam ID: " << examId << endl;
    }
}
time_t convertToEpoch(const string& dateTime) {
    tm timeStruct = {};
    istringstream ss(dateTime);
    char delimiter;
    ss >> timeStruct.tm_year >> delimiter >> timeStruct.tm_mon >> delimiter >> timeStruct.tm_mday
       >> timeStruct.tm_hour >> delimiter >> timeStruct.tm_min >> delimiter >> timeStruct.tm_sec;
    timeStruct.tm_year -= 1900;
    timeStruct.tm_mon -= 1;
    return mktime(&timeStruct);
}
int main() {
    Database db;
    int choice;
    string uname, pword;
    while (true) {
        cout << "\n--- Examination System ---\n";
        cout << "1. Login as Faculty\n2. Login as Student\n3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(); // To ignore the newline character after the integer input
        switch (choice) {
            case 1: {
                cout << "Enter Faculty username: ";
                getline(cin, uname);
                cout << "Enter Faculty password: ";
                getline(cin, pword);
                Faculty faculty1(uname,pword);
                if (faculty1.validateCredentials(db,"faculty")) {
                    faculty1.login();
                    int facultyChoice;
                    while (true) {
                        cout << "\n--- Faculty Menu ---\n";
                        cout << "1. Create Exam\n2. Delete Exam\n3. Add Question to Exam\n";
                        cout << "4. Update Question in Exam\n5. Delete Question from Exam\n";
                        cout << "6. Set/Update Exam Timings\n7. Show Exam\n8. Logout\n";
                        cout << "Enter your choice: ";
                        cin >> facultyChoice;
                        cin.ignore(); // Ignore newline
                        if (facultyChoice == 8) break;
                        switch (facultyChoice) {
                            case 1: { // Create Exam
                                int examId;
                                cout << "Enter Exam ID: ";
                                cin >> examId;
                                cin.ignore();
                                faculty1.createExam(examId);
                                break;
                            }
                            case 2: { // Delete Exam
                                int examId;
                                cout << "Enter Exam ID to delete: ";
                                cin >> examId;
                                cin.ignore();
                                faculty1.deleteExam(examId);
                                break;
                            }
                            case 3: { // Add Question to Exam
                                int examId, correctOption, marks;
                                string questionText;
                                vector<string> options(4);
                                cout << "Enter Exam ID: ";
                                cin >> examId;
                                cin.ignore();
                                cout << "Enter question text: ";
                                getline(cin, questionText);
                                cout << "Enter 4 options:" << endl;
                                for (int i = 0; i < 4; i++) {
                                    cout << "Option " << i + 1 << ": ";
                                    getline(cin, options[i]);
                                }
                                cout << "Enter the number of the correct option: ";
                                cin >> correctOption;
                                cout << "Enter marks for this question: ";
                                cin >> marks;
                                cin.ignore();
                                faculty1.addQuestionToExam(examId, questionText, options, correctOption, marks);
                                break;
                            }
                            case 4: { // Update Question in Exam
                                int examId, questionIndex, correctOption, marks;
                                string newQuestionText;
                                vector<string> newOptions(4);
                                cout << "Enter Exam ID: ";
                                cin >> examId;
                                cout << "Enter Question Number to update: ";
                                cin >> questionIndex;
                                cin.ignore();
                                cout << "Enter new question text: ";
                                getline(cin, newQuestionText);
                                cout << "Enter new 4 options:" << endl;
                                for (int i = 0; i < 4; i++) {
                                    cout << "Option " << i + 1 << ": ";
                                    getline(cin, newOptions[i]);
                                }
                                cout << "Enter the number of the new correct option: ";
                                cin >> correctOption;
                                cout << "Enter new marks for this question: ";
                                cin >> marks;
                                cin.ignore();
                                faculty1.updateQuestionInExam(examId, questionIndex, newQuestionText, newOptions, correctOption, marks);
                                break;
                            }
                            case 5: { // Delete Question from Exam
                                int examId, questionIndex;
                                cout << "Enter Exam ID: ";
                                cin >> examId;
                                cout << "Enter Question Number to delete: ";
                                cin >> questionIndex;
                                cin.ignore();
                                faculty1.deleteQuestionFromExam(examId, questionIndex);
                                break;
                            }
                            case 6: { // Set/Update Exam Timings
                                int examId;
                                string ns,ne;
                                time_t newStart, newEnd;
                                cout << "Enter Exam ID: ";
                                cin >> examId;
                                cin.ignore();
                                cout << "Enter new start time (YYYY-MM-DD HH:MM:SS): ";
                                getline(cin, ns);
                                cout << "Enter new end time (YYYY-MM-DD HH:MM:SS): ";
                                getline(cin, ne);
                                newStart=convertToEpoch(ns);
                                newEnd=convertToEpoch(ne);
                                faculty1.setExamTimings(examId, newStart, newEnd);
                                break;
                            }
                            case 7: { // Show Exam
                                int examId;
                                cout << "Enter Exam ID to show: ";
                                cin >> examId;
                                cin.ignore();
                                faculty1.showExam(examId);
                                break;
                            }
                            default:
                                cout << "Invalid choice!" << endl;
                                break;
                        }
                    }
                } else {
                    cout << "Invalid credentials!" << endl;
                }
                break;
            }
            case 2: {
                cout << "Enter Student username: ";
                getline(cin, uname);
                cout << "Enter Student password: ";
                getline(cin, pword);
                Student student1(uname,pword);
                Faculty faculty1("keerthika","k123");
                if (student1.validateCredentials(db,"student")) {
                    student1.login();
                    int studentChoice;
                    while (true) {
                        cout << "\n--- Student Menu ---\n";
                        cout << "1. Take Exam\n2. Update Answer\n3. View Result\n4. Logout\n";
                        cout << "Enter your choice: ";
                        cin >> studentChoice;
                        cin.ignore(); // Ignore newline
                        if (studentChoice == 4) break;
                        switch (studentChoice) {
                            case 1: { // Take Exam
                                int examId;
                                cout << "Enter Exam ID to take: ";
                                cin >> examId;
                                cin.ignore();
                                if (faculty1.examExists(examId)) {
                                	faculty1.randomizeQuestionOrder(examId);
                                    Exam& exam = faculty1.getExam(examId);
                                    student1.takeExam(exam,uname);
                                } else {
                                    cout << "Exam does not exist!" << endl;
                                }
                                break;
                            }
                             case 2: { // Update Answer
                                int examId, questionIndex, newAnswer;
                                cout << "Enter Exam ID: ";
                                cin >> examId;
                                cout << "Enter Question Number to update: ";
                                cin >> questionIndex;
                                cout << "Enter new answer option number: ";
                                cin >> newAnswer;
                                cin.ignore();

                                if (faculty1.examExists(examId)) {
                                    Exam& exam = faculty1.getExam(examId);
                                    student1.updateAnswer(examId, questionIndex, newAnswer, exam);
                                } else {
                                    cout << "Exam does not exist!" << endl;
                                }
                                break;
                            }
                            case 3: { // View Result
                                int examId;
                                cout << "Enter Exam ID to view result: ";
                                cin >> examId;
                                cin.ignore();
                                student1.viewResult(examId);
                                break;
                            }
                            default:
                                cout << "Invalid choice!" << endl;
                                break;
                        }
                    }
                } else {
                    cout << "Invalid credentials!" << endl;
                }
                break;
            }
            case 3:
                cout << "Exiting the system. Goodbye!" << endl;
                return 0;
            default:
                cout << "Invalid choice!" << endl;
                break;
        }
    }
    return 0;
}
