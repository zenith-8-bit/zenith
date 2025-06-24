import os
from flask import Flask, request, jsonify
import speech_recognition as sr
from pydub import AudioSegment
from werkzeug.utils import secure_filename
import tempfile

app = Flask(__name__)

# Configure upload folder and allowed extensions (for security and file handling)
# Although we're using temp files, defining allowed extensions is good practice.
ALLOWED_EXTENSIONS = {'wav', 'mp3', 'ogg', 'flac'}

def allowed_file(filename):
    """
    Checks if the uploaded file has an allowed extension.
    """
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def transcribe_audio(audio_file_path):
    """
    Transcribes the audio file to text using Google Speech Recognition.

    Args:
        audio_file_path (str): The path to the audio file.

    Returns:
        tuple: A tuple containing (transcribed_text, success_flag).
               Returns (text, True) on success, (None, False) on failure.
    """
    recognizer = sr.Recognizer()
    try:
        # Convert audio to WAV format if it's not already, as SpeechRecognition prefers WAV
        # and it handles various formats better with pydub.
        base, ext = os.path.splitext(audio_file_path)
        if ext.lower() not in ['.wav']:
            # Create a temporary WAV file path
            temp_wav_path = tempfile.mktemp(suffix=".wav")
            audio = AudioSegment.from_file(audio_file_path, format=ext[1:])
            audio.export(temp_wav_path, format="wav")
            audio_file_path = temp_wav_path # Use the converted WAV file for transcription

        with sr.AudioFile(audio_file_path) as source:
            # Adjust for ambient noise to improve recognition accuracy
            recognizer.adjust_for_ambient_noise(source)
            audio_data = recognizer.record(source) # Read the entire audio file

        # Perform speech recognition using Google Web Speech API
        text = recognizer.recognize_google(audio_data)
        print(f"Transcription successful: {text}")
        return text, True

    except sr.UnknownValueError:
        print("Google Speech Recognition could not understand audio.")
        return "Could not understand audio", False
    except sr.RequestError as e:
        print(f"Could not request results from Google Speech Recognition service; {e}")
        return f"Recognition service error: {e}", False
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return f"An unexpected error occurred: {e}", False
    finally:
        # Clean up the temporary WAV file if it was created
        if 'temp_wav_path' in locals() and os.path.exists(temp_wav_path):
            os.remove(temp_wav_path)


@app.route('/upload_audio', methods=['POST'])
def upload_audio():
    """
    Handles audio file uploads, transcribes them, and returns the text.
    """
    # Check if the POST request has the file part
    if 'audio' not in request.files:
        return jsonify({"success": False, "error": "No audio file part in the request"}), 400

    file = request.files['audio']

    # If the user does not select a file, the browser submits an
    # empty file without a filename.
    if file.filename == '':
        return jsonify({"success": False, "error": "No selected file"}), 400

    if file and allowed_file(file.filename):
        # Create a secure filename for the uploaded file
        filename = secure_filename(file.filename)
        # Create a temporary file to save the uploaded audio
        temp_dir = tempfile.gettempdir() # Get system's temporary directory
        temp_audio_path = os.path.join(temp_dir, filename)

        try:
            file.save(temp_audio_path) # Save the uploaded file temporarily
            print(f"Audio file saved temporarily at: {temp_audio_path}")

            # Transcribe the audio
            transcribed_text, success = transcribe_audio(temp_audio_path)

            if success:
                return jsonify({"success": True, "text": transcribed_text})
            else:
                return jsonify({"success": False, "error": transcribed_text})

        except Exception as e:
            print(f"Error processing file: {e}")
            return jsonify({"success": False, "error": f"Error processing file: {e}"}), 500
        finally:
            # Ensure the temporary file is deleted after processing
            if os.path.exists(temp_audio_path):
                os.remove(temp_audio_path)
                print(f"Temporary audio file deleted: {temp_audio_path}")
    else:
        return jsonify({"success": False, "error": "File type not allowed"}), 400

if __name__ == '__main__':
    # You can run the Flask app on localhost
    print("Starting Flask server...")
    print("Upload audio files via POST request to http://127.0.0.1:5000/upload_audio")
    print("Accepted audio formats: WAV, MP3, OGG, FLAC")
    app.run(debug=True) # debug=True allows for automatic reloading on code changes
