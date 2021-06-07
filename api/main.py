from flask import Flask, request, json, jsonify
from flask_pymongo import PyMongo
from marshmallow import Schema, fields, ValidationError
from bson.json_util import dumps
from json import loads
import datetime


app = Flask(__name__)
app.config["MONGO_URI"] = "mongodb+srv://supm:qwerty123@cluster0.ejobu.mongodb.net/lab3?retryWrites=true&w=majority"
mongo = PyMongo(app)


class List(Schema):
    patientID = fields.String(required = True)
    position = fields.Integer(required = True, strict = True)
    temp = fields.Integer(required = True, strict = True)

class List(Schema):
    fname = fields.String(required = True)
    lname = fields.String(required = True)
    age = fields.Integer(required = True, strict = True)
    patient_id = fields.String(requred = True)


@app.route("/api/record", methods = ["POST"])
def Entry_new():
  
    x = datetime.now()
    Rquest = request.json 
   
   
    temp_data = {
        "PatientID" : Rquest["patientID"],
        "Position" : Rquest["position"],
        "Temperature":Rquest["temp"]
    }
    
    try:
        Temp_rec = List().load(temp_data) 
        Temp_rec["last_updated"] = x
        mongo.db.record.insert_one(Temp_rec) 
        
        return {"success":True, "msg":"Data saved in temp_data successfully"}
    except ValidationError as ve:
        return ve.messages, 400 
    except Exception as e:   
        return {"msg" : "Failed"}, 500 


 
@app.route("/api/List")
def patients_entry():
    objAll = mongo.db.List.find()
    json_pat = jsonify(loads(dumps(objAll))) 
    return json_pat

@app.route('/api/List', methods = ['POST'])
def patient_new():
    Rquest = request.json
 
    temp_data = {
        "fname" : Rquest["fname"],
        "lname" : Rquest["lname"],
        "age" : Rquest["age"],
        "patient_id":Rquest["patient_id"]
    }  
    try:
        Ptemp = List().load(temp_data) 
        mongo.db.List.insert_one(Ptemp) 
        return {"success":True, "msg":"Data saved in temp_data successfully"}
    except ValidationError as ve:
        return ve.messages, 400 
    except Exception as e:
        return {"msg" : "Failed"}, 500 

@app.route('/api/List/<id>', methods = ["GET", "PATCH", "DELETE"])
def patient_names(id):
    ID_patient = mongo.db.List.find_one({"patient_id":id})
    json_pat = loads(dumps(ID_patient))
    if request.method == "GET":
        return json_pat
    elif request.method == "PATCH":
        Rquest = request.json
        if "fname" in Rquest:
            json_pat["fname"] = Rquest["fname"]
            mongo.db.List.update_one({"patient_id":id}, {"$set":{"fname":json_pat["fname"]}})
        if "lname" in Rquest:
            json_pat["lname"] = Rquest["lname"]
            mongo.db.List.update_one({"patient_id":id}, {"$set":{"lname": json_pat["lname"]}})
        if "age" in Rquest:
            json_pat["age"] = Rquest["age"]
            mongo.db.List.update_one({"patient_id":id}, {"$set":{"age": json_pat["age"]}})
        if "patient_id" in Rquest:
            json_pat["patient_id"] = Rquest["patient_id"]
            mongo.db.List.update_one({"patient_id":id}, {"$set":{"patient_id":json_pat["patient_id"]}})
      
        ID_patient = mongo.db.List.find_one({"patient_id":json_pat["patient_id"]})
        json_pat = loads(dumps(ID_patient))
        return json_pat
    elif request.method == "DELETE":
        Inst_del = mongo.db.List.delete_one({"patient_id":id})
        if Inst_del.deleted_count == 1:
            return {"success": True}
        else:
            return {"success": False}, 400

if __name__ == "__main__":
    app.run(debug = True, host="0.0.0.0", port = 5000)