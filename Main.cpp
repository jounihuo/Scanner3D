//Image processing OpenCV
#include<opencv2/opencv.hpp>
//OpenGL for rendering
#define GLEW_STATIC
#include<GL/glew.h>
#include<GLFW/glfw3.h>
//Serial communication with Arduino
#include<serial/serial.h>

#include<conio.h>
#include<fstream>
#include<stdio.h>
#include<cmath>
#include "thread"
#include<Windows.h>

using namespace cv;
using namespace std;

vector<double> slicing(vector<double>& arr, int X, int Y)
{
    // Starting and Ending iterators
    auto start = arr.begin() + X;
    auto end = arr.begin() + Y + 1;
    // To store the sliced vector
    vector<double> result(Y - X + 1);
    // Copy vector using copy function()
    copy(start, end, result.begin());
    // Return the final sliced vector
    return result;
}

void printVec(vector<double>& ans)
{
    // Traverse the vector ans
    for (auto& it : ans) {
        // Print elements
        cout << it << ' ';
    }
}

double tri_quality(double p0[3], double p1[3], double p2[3])
{
    // This function calculates the triangle quality by
    // the ratio of circumscribedand inscribed circles

    //Calculating triangles edge lengths
    double a = sqrt(pow(p0[0] - p1[0], 2) + pow(p0[1] - p1[1], 2) + pow(p0[2] - p1[2], 2));
    double b = sqrt(pow(p1[0] - p2[0], 2) + pow(p1[1] - p2[1], 2) + pow(p1[2] - p2[2], 2));
    double c = sqrt(pow(p2[0] - p0[0], 2) + pow(p2[1] - p0[1], 2) + pow(p2[2] - p0[2], 2));
    //Perimeter
    double p = a + b + c;
    //Half Perimeter
    double s = p / 2;
    //Area using Heron's formula
    double A = sqrt(s * (s - a) * (s - b) * (s - c));
    //Radius of circle inside the triangle
    double r_inner = 2 * A / p;
    //Radius of circle outside the triangle
    double r_outer = a * b * c / (4 * A);

    return r_outer / r_inner;
}

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 800;

// Shaders
const GLchar* vertexShaderSource = "#version 450 core\n"
"layout (location = 0) in vec3 position;\n"
"out float Zposition;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"Zposition = position.z;\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 450 core\n"
"in float Zposition;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"float scale = abs(float(Zposition)+0.5)  ;\n"
"color = vec4(scale*0.1, scale, scale*0.1, 0.2f);\n"
"}\n\0";

int main()
{
    //**************************************************************
    // ****** This part initializes the serial communication *******
    // *************************************************************
    serial::Serial my_serial("COM6", 57600, serial::Timeout::simpleTimeout(3000));
    if (my_serial.isOpen())
    {
        std::cout << "Port opened succesfully" << std::endl;
    }
    else
    {
        std::cout << "Port failed to open" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    my_serial.flushInput();

    std::string one_string = "0";
    std::string test_string = "GO\n";
    std::string str_ready("READY");

    my_serial.write(one_string);
    my_serial.write(one_string);

    char ch;
    while (1) {
        cout << "Press 's' to start..." << endl;
        ch = _getch();
        if (ch == 's') {
            my_serial.write(test_string);
            break;
        }
        else {

        }
    }
    //**************************************************************
    // ********* This part initializes the 3D rendering ************
    // *************************************************************
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Point Cloud Render", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Build and compile our shader program
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for compile time errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for compile time errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    //**************************************************************
    // ********* This part contains the OpenCV pipeline ************
    // *************************************************************

    //Point cloud coordinates
    vector<double> xCoord;
    vector<double> yCoord;
    vector<double> zCoord;
    vector<int> nCount;

    vector<int> triIDS;
    int nTrias =  0;
    int scaleImg = 50;

    double ANG = 3.0;
    // converting degrees to radians
    ANG = ANG * 3.14159 / 180;

    // Create a VideoCapture object and open the input file
    // If the input is the web camera, pass 0 instead of the video file name
    //VideoCapture cap("cv_monkey_e.mp4");
    VideoCapture cap(0);

    nCount.push_back(0);
    // Check if camera opened successfully
    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    int nScan = 0;
    Mat frame;
    ofstream myfile;
    while (1) {
        // Telling the Arduino to take a step forward
        my_serial.write(test_string);
        my_serial.flushInput();
        while (1) {
            // Reading response from Arduino 
            std::string response = my_serial.read(5);
            std::cout << "Arduino: " << response << std::endl;
            int res = str_ready.compare(response);
            // Stops waiting when READY signal is received from Arduino
            if (res == 0) break;
        }

        // Capture frame-by-frame
        cap >> frame;

        // If the frame is empty, break immediately
        if (frame.empty()){
            break;
        }

        //Scale the scanned image
        //resize(frame, frame, Size(), 0.5, 0.5, INTER_LINEAR);
        GaussianBlur(frame, frame, Size(5, 5), 0);

        // RGB image to HSV
        Mat hsv_image; 
        cvtColor(frame, hsv_image, COLOR_BGR2HSV);

        // Masking
        Mat mask_image;
        Scalar mask_low = Scalar(0, 150, 150);
        Scalar mask_high = Scalar(200, 255, 255);
        inRange(hsv_image, mask_low, mask_high, mask_image);
        Mat output;
        bitwise_and(frame, frame, output, mask_image);

        // Profile calculation
        Vec3b pixel;
        int nPix = 0; //Number of pixels with non zero values
        int nSum = 0; //Sum of the pixel locations
        int nPro = 0;
        vector<int> profile_x;
        vector<int> profile_y;
        vector<int> profile_z;

        for (int i = 0; i < output.rows; i++){
            nPix = 0;
            nSum = 0;
            //Collect non zeros pixel locations
            for (int j = 0; j < output.cols; j++) {
                pixel = output.at<Vec3b>(i, j);
                if (pixel[0]+pixel[1]+pixel[2] > 0) {
                    nPix = nPix + 1;
                    nSum = nSum + j;
                }
            }
            //Calculate the mean pixel location
            if (nPix>0){
                profile_x.push_back((nSum / nPix) - output.size[1]/2);
                profile_y.push_back(i);
                nPro++;
            }

        }
        nCount.push_back(nPro);
        profile_z.assign(profile_x.size(), 0);

        //Collecting the coordinates
        xCoord.insert(end(xCoord), begin(profile_x), end(profile_x));
        yCoord.insert(end(yCoord), begin(profile_y), end(profile_y));
        zCoord.insert(end(zCoord), begin(profile_z), end(profile_z));

        //3D rotation 
        vector<double> xTemp = xCoord;
        vector<double> zTemp = zCoord;
        for (int i = 0; i < zCoord.size(); ++i) {
            xCoord[i] = zTemp[i] * sin(ANG) + xTemp[i] * cos(ANG);
            zCoord[i] = zTemp[i] * cos(ANG) - xTemp[i] * sin(ANG);
        }
 
        //Creating a mesh between the scan lines
        if (nCount.size() > 2) {
            int i2 = xCoord.size() - 1;
            int i1 = i2 - nCount[int(nCount.size() - 1)];
            int i0 = i1 - nCount[int(nCount.size() - 2)] + 1;
            
            int hmm = 5;

            vector<double> slice1x, slice1y, slice1z;
            slice1x = slicing(xCoord, i0 + hmm, i1- hmm);
            slice1y = slicing(yCoord, i0 + hmm, i1- hmm);
            slice1z = slicing(zCoord, i0 + hmm, i1- hmm);

            vector<double> slice2x, slice2y, slice2z;
            slice2x = slicing(xCoord, i1 + hmm, i2 - hmm);
            slice2y = slicing(yCoord, i1 + hmm, i2 - hmm);
            slice2z = slicing(zCoord, i1 + hmm, i2 - hmm);

            // Vector max indices
            int na = slice1x.size() - 1;
            int nb = slice2x.size() - 1;
            // Starting indices
            int a0 = 0;
            int b0 = 0;
            int a_offset = i0;
            int b_offset = i1+1;
            // Looping throug all
            while ((a0 < na) || (b0 < nb)) {
                //Selecting two point for the triangle
                double p0[3] = { slice1x[a0], slice1y[a0], slice1z[a0] };
                double p1[3] = { slice2x[b0], slice2y[b0], slice2z[b0] };
                double p2a[3], p2b[3];
                //Selecting possible third point
                //Checking if already at the end of list
                if (a0 < na){
                    p2a[0] = slice1x[a0 + 1];
                    p2a[1] = slice1y[a0 + 1];
                    p2a[2] = slice1z[a0 + 1];
                }
                else {
                    p2a[0] = slice1x[a0] + 1.0e-6;
                    p2a[1] = slice1y[a0] + 1.0e-6;
                    p2a[2] = slice1z[a0] + 1.0e-6;
                }
                if (b0 < nb) {
                    p2b[0] = slice2x[b0 + 1];
                    p2b[1] = slice2y[b0 + 1];
                    p2b[2] = slice2z[b0 + 1];
                }
                else {
                    p2b[0] = slice2x[b0] + 1.0e-6;
                    p2b[1] = slice2y[b0] + 1.0e-6;
                    p2b[2] = slice2z[b0] + 1.0e-6;
                }
                //Selecting the third point based on triangle quality
                if (tri_quality(p0, p1, p2a) < tri_quality(p0, p1, p2b)) {
                    //Adding triangle to results
                    triIDS.push_back(b0 + b_offset);     //3
                    triIDS.push_back(a0 + a_offset);     //2
                    triIDS.push_back(a0 + 1 + a_offset); //1
                    nTrias++;
                    a0++;
                }
                else {
                    //Adding triangle to results
                    triIDS.push_back(b0 + b_offset + 1);  //3
                    triIDS.push_back(b0 + b_offset);      //2
                    triIDS.push_back(a0 + a_offset);      //1
                    nTrias++;
                    b0++;
                }
            }
        }
        
        //Overlay extracted profile to image
        int thickness = 1;
        for (int i = 0; i < nPro - 1; i++) {
            Point p1(profile_x[i] + output.size[1] / 2, profile_y[i]);
            Point p2(profile_x[i + 1] + output.size[1] / 2, profile_y[i + 1]);
            // Line drawn
            line(frame, p1, p2, Scalar(255, 255, 255), thickness, LINE_8);
        }
        
        // Display the resulting frame
        imshow("Scan window", frame);

        // Press  ESC on keyboard to exit
        char c = (char)waitKey(25);
        if (c == 27)
            break;

        // ***************************************************************
        // *** This part contains the rendering of the meshed surface ****
        // ***************************************************************
        
        // Finding out the scaling factor for vertices 
        double maxX = *max_element(xCoord.begin(), xCoord.end());
        double minX = *min_element(xCoord.begin(), xCoord.end());
        double maxY = *max_element(yCoord.begin(), yCoord.end());
        double minY = *min_element(yCoord.begin(), yCoord.end());
        double maxZ = *max_element(zCoord.begin(), zCoord.end());
        double minZ = *min_element(zCoord.begin(), zCoord.end());

        double scale = abs(maxX - minX);
        if (abs(maxY - minY) > scale) scale = abs(maxY - minY);
        if (abs(maxZ - minZ) > scale) scale = abs(maxZ - minZ);

        scale = scale * 0.4;

        // Set up vertex data (and buffer(s)) and attribute pointers
        GLfloat* verticesXYZt = new GLfloat[1500000];

        int i = 0;
        for (int idx = 0; idx < 3*nTrias; ++idx) {
            verticesXYZt[i + 0] = (float)(xCoord[triIDS[idx]] / scale);
            verticesXYZt[i + 1] = (float)((yCoord[triIDS[idx]] / (2 * scale) - 0.75) * -1.);
            verticesXYZt[i + 2] = (float)(zCoord[triIDS[idx]] / scale);
            i = i + 3;
        }
      
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 4*(i-3), verticesXYZt, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
        glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.4f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the points
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3*nTrias-1);

        // Swap the screen buffers
        glfwSwapBuffers(window);

        nScan = nScan + 1;
        cout << nScan << endl;
        if (nScan > 119) {
            break;
        }
    }
    // Write STL-file
    myfile.open("example.stl");
    myfile << "solid\n";
    for (int i = 0; i < triIDS.size(); i = i + 3) {
        myfile << "facet normal 0 0 1\n";
        myfile << "outer loop\n";
        myfile << "vertex " << xCoord[triIDS[i]] << " " << yCoord[triIDS[i]] / 2 << " " << zCoord[triIDS[i]] << "\n";
        myfile << "vertex " << xCoord[triIDS[i+1]] << " " << yCoord[triIDS[i+1]] / 2 << " " << zCoord[triIDS[i+1]] << "\n";
        myfile << "vertex " << xCoord[triIDS[i+2]] << " " << yCoord[triIDS[i+2]] / 2 << " " << zCoord[triIDS[i+2]] << "\n";
        myfile << "endloop\n";
        myfile << "endfacet\n";
    }
    myfile << "endsolid\n";
    myfile.close();
    cout << "STL written." << endl;

    my_serial.write(one_string);
    // When everything done, release the video capture object
    cap.release();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    // Closes all windows
    destroyAllWindows();

    cout << "All done. Thank you." << endl;

    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}