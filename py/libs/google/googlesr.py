# coding: utf8
# googlesr.py 11/1/2014
# See: https://www.google.com/intl/ja/chrome/demos/speech.html
# See: https://pypi.python.org/pypi/SpeechRecognition/

if __name__ == '__main__':
  import speech_recognition as sr
  r = sr.Recognizer(language='ja')
  with sr.Microphone() as source:                # use the default microphone as the audio source
    print "listen start"
    audio = r.listen(source)                   # listen for the first phrase and extract it into audio data
    print "listen stop"

  try:
    print("You said " + r.recognize(audio))    # recognize speech using Google Speech Recognition
  except LookupError:                            # speech is unintelligible
    print("Could not understand audio")

# EOF
