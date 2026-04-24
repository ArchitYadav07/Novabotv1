import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

const firebaseConfig = {
  apiKey: "Fk0eUhJhyOGEeJAoFuGEnd1XnXyKqx1Wu5IlSb5q6bE",
  authDomain: "novabot-298e9.firebaseapp.com",
  databaseURL: "https://novabot-298e9-default-rtdb.firebaseio.com",
  projectId: "novabot-298e9",
  storageBucket: "novabot-298e9.appspot.com",
  messagingSenderId: "515915915915", // Placeholder
  appId: "1:515915915915:web:515915915915" // Placeholder
};

const app = initializeApp(firebaseConfig);
export const db = getDatabase(app);
