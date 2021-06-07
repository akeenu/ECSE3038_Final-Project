# ECSE3038_Final-Project
This project consist of an api, arduino sketch and frontend.
The backend application should be able to:

- GET /api/patient

    This route should return an array of all patient objects stored in the database.

- GET /api/patient/:id

    This route should return a single patient object. The patient returned should have an ID that matches the one specified in the request.

- POST /api/patient

    This route should handle the creation of a new patient object. The parameters of this object should be taken from the JSON body of the incoming request.

- PATCH /api/patient/:id

    This route should allow a user to edit any of the details of a specified patient object identified by their ID. The route should respond with the newly edited patient object.

- DELETE /api/patient/:id

    This route should allow a user to delete a specified patient object identified by their ID. The route should respond with a message that indicates weather the operation was successful.

The frontend should have two views, one which displays a listing of patients' names and the second view for detailing information on the patient.
