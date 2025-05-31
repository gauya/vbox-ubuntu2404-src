from fastapi import FastAPI, File, UploadFile, Form
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
import os

app = FastAPI()

# Mount static files directory for uploaded files
app.mount("/static", StaticFiles(directory="static"), name="static")

# HTML content for the split-screen interface
html_content = """
<!DOCTYPE html>
<html>
<head>
    <title>Query and File Info App</title>
    <style>
        body, html {
            margin: 0;
            padding: 0;
            height: 100%;
            font-family: Arial, sans-serif;
        }
        .container {
            display: flex;
            height: 100vh;
        }
        .left, .right {
            width: 50%;
            padding: 20px;
            box-sizing: border-box;
            overflow-y: auto;
        }
        .left {
            background-color: #f0f0f0;
        }
        .right {
            background-color: #e0e0e0;
        }
        textarea, input[type="text"], input[type="file"] {
            width: 100%;
            margin-bottom: 10px;
            padding: 10px;
        }
        button {
            padding: 10px 20px;
            background-color: #007bff;
            color: white;
            border: none;
            cursor: pointer;
            margin-right: 10px;
        }
        button:hover {
            background-color: #0056b3;
        }
        #response, #file-info {
            margin-top: 20px;
            padding: 10px;
            background-color: white;
            border-radius: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="left">
            <h2>Query</h2>
            <form id="query-form">
                <textarea id="query" name="query" rows="5" placeholder="Enter your query"></textarea>
                <button type="submit">Submit Query</button>
            </form>
            <h2>File Upload</h2>
            <form id="file-form" enctype="multipart/form-data">
                <input type="file" id="file" name="file">
                <button type="submit">Upload File</button>
            </form>
        </div>
        <div class="right">
            <h2>Query Response</h2>
            <div id="response">No response yet.</div>
            <h2>File Information</h2>
            <div id="file-info">No file selected.</div>
        </div>
    </div>
    <script>
        // Handle query form submission
        document.getElementById('query-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            const query = document.getElementById('query').value;
            const responseDiv = document.getElementById('response');
            try {
                const response = await fetch('/query', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: new URLSearchParams({ 'query': query })
                });
                const data = await response.json();
                responseDiv.innerText = data.response;
            } catch (error) {
                responseDiv.innerText = 'Error: ' + error.message;
            }
        });

        // Handle file upload
        document.getElementById('file-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            const fileInput = document.getElementById('file');
            const fileInfoDiv = document.getElementById('file-info');
            const formData = new FormData();
            formData.append('file', fileInput.files[0]);
            try {
                const response = await fetch('/upload', {
                    method: 'POST',
                    body: formData
                });
                const data = await response.json();
                fileInfoDiv.innerHTML = `File Name: ${data.filename}<br>File Size: ${data.size} bytes<br>File Type: ${data.content_type}`;
            } catch (error) {
                fileInfoDiv.innerText = 'Error: ' + error.message;
            }
        });
    </script>
</body>
</html>
"""

# Create static directory if it doesn't exist
if not os.path.exists("static"):
    os.makedirs("static")

@app.get("/", response_class=HTMLResponse)
async def get_root():
    return html_content

@app.post("/query")
async def process_query(query: str = Form(...)):
    # Simple echo response for demonstration; replace with actual query processing logic
    return {"response": f"Received query: {query}"}

@app.post("/upload")
async def upload_file(file: UploadFile = File(...)):
    # Save the file to the static directory
    file_path = f"static/{file.filename}"
    with open(file_path, "wb") as f:
        f.write(file.file.read())
    return {
        "filename": file.filename,
        "size": file.size,
        "content_type": file.content_type
    }
