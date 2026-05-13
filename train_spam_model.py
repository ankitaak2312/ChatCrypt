import pandas as pd
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.naive_bayes import MultinomialNB
from sklearn.pipeline import Pipeline
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import joblib

# Dataset
df = pd.read_csv("spam.csv", encoding='latin-1')

# Keep needed columns
df = df[['v1', 'v2']]
df.columns = ['label', 'message']

# Convert labels
df['label'] = df['label'].map({'ham': 0, 'spam': 1})

X = df['message']
y = df['label']

# Split
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# Pipeline
model = Pipeline([
    ('tfidf', TfidfVectorizer()),
    ('clf', MultinomialNB())
])

# Train
model.fit(X_train, y_train)

# Test
pred = model.predict(X_test)

print("Accuracy:", accuracy_score(y_test, pred))

# Save
joblib.dump(model, "spam_model.pkl")