// require models
const sequelize = require('../util/database');
var initModels = require("../models/init-models");
var models = initModels(sequelize);
// end of require models

module.exports = (req, res, next) => {
    let token = req.headers['x-observatory-auth'] || req.headers['authorization'];

        if (!token) {
            return res.status(500).json({
            success: false,
            message: 'Auth token is not supplied'
          })
        }
      
        if (token) {
            models.expired_tokens.findOne({ where: {token: token} }).
            then(expired => {

                if (expired) return res.status(400).json({ message: "You are already logged out." })
                else {
                    models.expired_tokens.create({ token: token })
                    return res.status(200).json({});
                }
            })
        }
}