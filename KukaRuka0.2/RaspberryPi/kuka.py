import time
import numpy as np
import cv2
import imutils
from pyzbar import pyzbar
import serial
# import matplotlib.pyplot as plt


ser = serial.Serial('/dev/ttyACM0', 57600) # Serial takes these two parameters: serial device and baudrate


def detectShapes(c):
    shape = "unidentified"
    peri = cv2.arcLength(c, True)
    approx = cv2.approxPolyDP(c, 0.04 * peri, True)
    if len(approx) == 3:
        shape = "triangle"
    elif len(approx) == 4:
        (x, y, w, h) = cv2.boundingRect(approx)
        ar = w / float(h)
        shape = "square" if ar >= 0.95 and ar <= 1.05 else "rectangle"
    elif len(approx) == 5:
        shape = "pentagon"
    else:
        shape = "circle"
    return shape

def getShapes(image):
    resized = imutils.resize(image, width=100)
    if display:
        cv2.imshow("Resized", resized)
    ratio = image.shape[0] / float(resized.shape[0])
    blurred = cv2.GaussianBlur(resized, (5, 5), 0)
    gray = cv2.cvtColor(blurred, cv2.COLOR_BGR2GRAY)
    lab = cv2.cvtColor(blurred, cv2.COLOR_BGR2LAB)
    thresh = cv2.threshold(gray, 60, 255, cv2.THRESH_BINARY)[1]
    cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
   
    if len(cnts) > 1:
        print "can't recognize shape"
        return
    for c in cnts:
        M = cv2.moments(c)
        if M["m00"] > 0:
            cX = int((M["m10"] / M["m00"]) * ratio)
            cY = int((M["m01"] / M["m00"]) * ratio)
            shape = detectShapes(c)
            print shape
       
    return


def rgbToHsv(r, g, b):
    r, g, b = r/255.0, g/255.0, b/255.0
    mx = max(r,g,b)
    mn = min(r,g,b)
    df = mx-mn
    if mx == mn:
        h = 0
    elif mx == r:
        h = (60 * ((g-b)/df) + 360) % 360
    elif mx == g:
        h = (60 * ((b-r)/df) + 120) % 360
    elif mx == b:
        h = (60 * ((r-g)/df) + 240) % 360
   
    if mx == 0:
        s = 0
    else:
        s = (df/mx)*100
       
    v = mx*100
    print "hsv: ", h, s, v
    return h, s, v

def getColor(r, g, b):
    if r < 50 and g < 50 and b < 50:
        return "n"
    if g > r and g > b:
        return "g"
    if b > r and b > g:
        return "b"
    if r > g*2:
        return "r"
    return "y"

def recognize():
    print("start recognizing")
    # get all barcodes
    bcT = "0"
    bcD = "0000000000"
    barcodes = pyzbar.decode(img)
    for bc in barcodes:
        bcData = bc.data.decode("utf8")
        bcType = bc.type
        if bcType == "QRCODE":
            bcT = "2"
        else:
            bcT = "1"
        bcD = bcData
    if len(bcD) > 10:
        bcD = bcD[0] + bcD[1] + bcD[2] + bcD[3] + bcD[4] + bcD[5] + bcD[6] + bcD[7] + bcD[8] + bcD[9]
    while len(bcD) < 10:
        bcD = "0" + bcD

    flipped = cv2.flip(img, 0)
    barcodes = pyzbar.decode(flipped)
    for bc in barcodes:
        bcData = bc.data.decode("utf8")
        bcType = bc.type
        if bcType == "QRCODE":
            bcT = "2"
        else:
            bcT = "1"
        bcD = bcData
    if len(bcD) > 10:
        bcD = bcD[0] + bcD[1] + bcD[2] + bcD[3] + bcD[4] + bcD[5] + bcD[6] + bcD[7] + bcD[8] + bcD[9]
    while len(bcD) < 10:
        bcD = "0" + bcD

    flipped = cv2.flip(img, -1)
    barcodes = pyzbar.decode(flipped)
    for bc in barcodes:
        bcData = bc.data.decode("utf8")
        bcType = bc.type
        if bcType == "QRCODE":
            bcT = "2"
        else:
            bcT = "1"
        bcD = bcData
    if len(bcD) > 10:
        bcD = bcD[0] + bcD[1] + bcD[2] + bcD[3] + bcD[4] + bcD[5] + bcD[6] + bcD[7] + bcD[8] + bcD[9]
    while len(bcD) < 10:
        bcD = "0" + bcD
        
    flipped = cv2.flip(img, 1)
    barcodes = pyzbar.decode(flipped)
    for bc in barcodes:
        bcData = bc.data.decode("utf8")
        bcType = bc.type
        if bcType == "QRCODE":
            bcT = "2"
        else:
            bcT = "1"
        bcD = bcData
    if len(bcD) > 10:
        bcD = bcD[0] + bcD[1] + bcD[2] + bcD[3] + bcD[4] + bcD[5] + bcD[6] + bcD[7] + bcD[8] + bcD[9]
    while len(bcD) < 10:
        bcD = "0" + bcD

    print bcT, bcD

    # get color of center region
    cropped = img[height/2+50:height/2+150, width/2-20:width/2+20]
    if display:
        cv2.imshow("Cropped", cropped)
    data = np.reshape(cropped, (-1,3))
    data = np.float32(data)
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 10, 1.0)
    flags = cv2.KMEANS_RANDOM_CENTERS
    compactness, labels, centers = cv2.kmeans(data, 1, None, criteria, 10, flags)
    b = centers[0][0].astype(np.int32)
    g = centers[0][1].astype(np.int32)
    r = centers[0][2].astype(np.int32)
    print "rgb: ", r, g, b, getColor(r,g,b)
    
    hsvL = np.array([0,0,0])
    hsvD = np.array([0,0,0])
    hsvL[0],hsvL[1],hsvL[2] = rgbToHsv(r,g,b)
    hsvC = cv2.cvtColor(np.uint8([[[b,g,r]]]), cv2.COLOR_BGR2HSV)
    hsvL[0],hsvL[1],hsvL[2] = hsvC[0][0][0],hsvC[0][0][1],hsvC[0][0][2]
    hsvD[0],hsvD[1],hsvD[2] = hsvC[0][0][0],hsvC[0][0][1],hsvC[0][0][2]
   
   
    hsvL[0] = hsvL[0] - 20
    if hsvL[0] < 0:
        hsvL[0] = 180 - hsvL[0]
    hsvD[0] = hsvD[0] + 20
    if hsvD[0] > 255:
        hsvD[0] = 255
    hsvL[1] = hsvL[1] - 50
    if hsvL[1] < 0:
        hsvL[1] = 0
    hsvD[1] = hsvD[1] + 50
    if hsvD[1] > 255:
        hsvD[1] = 255
    hsvL[2] = hsvL[2] - 50
    if hsvL[2] < 0:
        hsvL[2] = 0
    hsvD[2] = hsvD[2] + 50
    if hsvD[2] > 255:
        hsvD[2] = 255
       
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    hsvmask = cv2.inRange(hsv, (hsvL[0], hsvL[1], hsvL[2]), (hsvD[0], hsvD[1], hsvD[2]))
    hued = cv2.bitwise_and(img, img, mask=hsvmask)
    if display:
        cv2.imshow("ImageColor", hued)
   
    getShapes(hued)
   
    msg = getColor(r,g,b)+bcT+bcD+"\r\n"
    print msg
    ser.write(msg.encode())


# START PROGRAMM
if "-nd" in str(sys.argv):
    display = false;
stri = ''
cameraPort = 0
camera = cv2.VideoCapture(cameraPort)
time.sleep(0.1)
returnValue, img = camera.read()
height, width, channels = img.shape
print width, height
cv2.startWindowThread()
if display:
    cv2.namedWindow("Image")
    cv2.namedWindow("ImageColor")
    cv2.namedWindow("Cropped")
    cv2.namedWindow("Resized")
while True:
    returnValue, img = camera.read()
    if display:
        cv2.imshow("Image", img)
    if ser.inWaiting() > 0:
        data = ser.read()
        stri = stri + data.decode()
        if len(stri) > 2 and stri[-1] in '\r\n' and stri[-2] in '\r\n':
            print(stri)
            if stri == 'recognize\r\n':
                recognize()
            if stri == 'bye\r\n':
                print('exit')
                break
            stri = ''

# END OF PROGRAMM  =>
camera.release()
cv2.destroyAllWindows()
ser.close()
