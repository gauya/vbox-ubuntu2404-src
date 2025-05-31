from fastapi import FastAPI, File, UploadFile, Form
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.requests import Request
import os

app = FastAPI()

# Mount static files directory for uploaded files
app.mount("/static", StaticFiles(directory="static"), name="static")

# Set up Jinja2 templates
templates = Jinja2Templates(directory="templates")

# Create static directory if it doesn't exist
if not os.path.exists("static"):
    os.makedirs("static")
if not os.path.exists("templates"):
    os.makedirs("templates")

@app.get("/", response_class=HTMLResponse)
async def get_root(request: Request):
    return templates.TemplateResponse("frame.html", {"request": request})

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
