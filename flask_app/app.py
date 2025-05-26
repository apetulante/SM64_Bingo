# app.py
import os
import subprocess
import json
from flask import Flask, render_template, request, jsonify
from flask_cors import CORS
from flask import send_from_directory, redirect
from werkzeug.utils import secure_filename

app = Flask(__name__)
CORS(app)

# Ensure the data directory exists
DATA_DIR = os.path.join(os.path.dirname(__file__), 'data')
os.makedirs(DATA_DIR, exist_ok=True)

@app.route('/')
def index():
    # Get list of txt files in the data directory
    txt_files = [f for f in os.listdir(DATA_DIR) if f.endswith('.txt')]
    return render_template('index.html', txt_files=txt_files)

@app.route('/generate', methods=['POST'])
def generate_board():
    # Get form data
    input_file = request.form.get('input_file', './data/16star.txt')
    iterations = request.form.get('iterations', '1000000000')
    tolerance = request.form.get('tolerance', '1.0')
    seed = request.form.get('seed', '0')
    min_difficulty = request.form.get('min_difficulty', '0.0')

    # Construct full path to input file
    full_input_path = os.path.join(DATA_DIR, input_file)

    # Construct command
    cmd = [
        '../make_single_board',
        '--input', full_input_path,
        '--iterations', iterations,
        '--tolerance', tolerance,
        '--seed', seed,
        '--minDifficulty', min_difficulty
    ]

    try:
        # Run the generator
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=40)

        # Parse the output
        lines = result.stdout.split('\n')
        
        # Find the lines with row sums
        hardest_row = next((line for line in lines if 'Hardest Sum' in line), 'No hardest row found')
        easiest_row = next((line for line in lines if 'Easiest Sum' in line), 'No easiest row found')

        # Find the JSON output
        json_line = next((line for line in lines if line.startswith('[') and line.endswith(']')), 'Failed')
        
        # Parse the JSON
        try:
            board_json = json.loads(json_line)

            return jsonify({
                'hardest_row': hardest_row,
                'easiest_row': easiest_row,
                'board_json': board_json
            })
        except json.JSONDecodeError:
            return jsonify({
                'hardest_row': hardest_row,
                'easiest_row': easiest_row,
                'board_json': 'Failed to make board, please try again'
            })



    except subprocess.TimeoutExpired:
        return jsonify({'error': 'Generation timed out'}), 500
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/upload', methods=['POST'])
def upload():
    if 'custom_file' not in request.files:
        return redirect('/')

    file = request.files['custom_file']
    if file.filename == '':
        return redirect('/')

    if file and file.filename.endswith('.txt'):
        filename = secure_filename(file.filename)
        file.save(os.path.join(DATA_DIR, filename))

    return redirect('/')

if __name__ == '__main__':
    app.run(debug=True)