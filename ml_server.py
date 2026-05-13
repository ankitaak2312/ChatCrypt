from flask import Flask, request, jsonify
from transformers import pipeline
import joblib

app = Flask(__name__)

# Sentiment model
sentiment_model = pipeline("sentiment-analysis")

# Spam model
spam_model = joblib.load("spam_model.pkl")

@app.route('/analyze', methods=['POST'])
def analyze():

    data = request.json

    text = data['message']

    # sentiment
    sentiment = sentiment_model(text)[0]

    # spam prediction
    spam_prediction = spam_model.predict([text])[0]

    return jsonify({
        "sentiment": str(sentiment['label']),
        "confidence": float(sentiment['score']),
        "spam": int(spam_prediction)
    })

if __name__ == '__main__':
    app.run(port=5000)