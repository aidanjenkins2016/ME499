#!/Users/ryanyu/.virtualenvs/cv/bin/python

import cv2
import numpy as np

face_cascade = cv2.CascadeClassifier('haarcascade_frontalface.xml')

eye_cascade = cv2.CascadeClassifier('haarcascade_eye.xml')

test1 = face_cascade.load('haarcascade_frontalface.xml')
test2 = face_cascade.load('haarcascade_eye.xml')

print(test1)
print(test2)
