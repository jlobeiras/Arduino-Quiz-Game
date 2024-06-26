# Arduino-Quiz-Game
Device code for an Arduino based quiz game using a barcode scanner.

<img src="img/Barcode1.jpg" width="500">

Use an Arduino UNO with a USB host shield installed and connect a passive buzzer module to pin 9.
You can change the pin in the source code our update the different melodies.
You will also need a USB barcode reader that uses a normal keyboard HID interface.

**[ Arduino code ]**

See [QuizGame.ino](QuizGame.ino) file for microcontroller source code.

**[ Circuit Diagram ]**

<img src="img/ArduinoScannerCircuit.jpg" width="500">

Note: some USB Host shield modules do not have the power selection jumpers configured, in such cases you should use a soldering iron to bridge the three jumper points marked by the two yellow circles, otherwise the USB device will receive no power from the shield.

**[ Question cards ]**

Four types of questions are supported:
1. Single correct answer: The barcodes should have 4 digits following the pattern 1ABC, with A+B+C an odd number for correct anser.
2. Multiple correct answers: The barcodes should follow the pattern 2ABC with A=numberOfCorrectAnswers, B=2<sup>x</sup> for correct and not power of 2 for incorrect answer, C=constant (different for each card).
3. Match pairs of terms: The barcodes should be of the form 3A00 for the first term (with A=termIdentifier) and 30BC for the second term (with A=|B-C| for the correct answer).
4. Sort the answers: The barcodes should be of the form 4ABC, with A=numberOfElements and B+C should be a sequence from 6 to A+5.

See [ExampleQuestionCards.pdf](ExampleQuestionCards.pdf) file for some question card examples.

<a href="ExampleQuestionCards.pdf"><img src="img/ExampleSortCard.jpg" width="500"></a>

**[ Demo video ]**

Video showing the project and the different types of question cards (in galician language):

https://www.youtube.com/watch?v=I-SmN0XP7is
