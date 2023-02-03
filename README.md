# Scanner3D
c++ code for scanning a 3D surface from a captured image

The code requires to have the following dependencies available:
* OpenCV (https://opencv.org/)
* GLFW (https://www.glfw.org/)
* GLEW (https://glew.sourceforge.net/)
* Serial (for Arduino communication, https://github.com/wjwwood/serial)

OpenCV creates profile from a webcam image and GLFW is used to render the 3D reconstruction of the surface. Serial communication is used with arduino to rotate the scanned object in front of the webcam. A line laser is used to illuminate the object in 30 degree angle respect to the camera.
