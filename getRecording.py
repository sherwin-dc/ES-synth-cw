import serial
from serial import SerialException
import wave

try:
  ser = serial.Serial('COM5', 460800)
  ser.read_until(expected=b"RECSTART")
  print("Recording Start")
  recording = ser.read_until(expected=b"RECEND")
  print("Recording End")
  ExpectedSamples =int.from_bytes(recording[-10:-6],'little')
  
  recording = recording[0:-10]
  print(ExpectedSamples)
  print(len(recording)/600)

  correctedSampleRate = 22000 / ( ExpectedSamples / ( len(recording) / 600 ))


  wavfile = wave.open("rec.wav", 'w')
  wavfile.setnchannels(1)
  wavfile.setsampwidth(2)
  wavfile.setframerate(correctedSampleRate)
  wavfile.writeframesraw(recording)
  wavfile.close()


except SerialException:
  print("Exception. Check if the port is free")

finally:
  ser.close()
