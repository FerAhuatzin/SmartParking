
import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database"; // Para Realtime Database

const firebaseConfig = {
  apiKey: "AIzaSyAU19O3Vpbzz9TWRI-qvBkDJlRr_1_ztNI",
  authDomain: "smart-parking-ca5f8.firebaseapp.com",
  databaseURL: "https://smart-parking-ca5f8-default-rtdb.firebaseio.com",
  projectId: "smart-parking-ca5f8",
  storageBucket: "smart-parking-ca5f8.firebasestorage.app",
  messagingSenderId: "1096990903956",
  appId: "1:1096990903956:web:6c103ad651d7fa2a6bae81",
  measurementId: "G-NLZEYRNEYF"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);
console.log("Firebase initialized", app.name);
console.log("Database initialized", database);
export { database };
