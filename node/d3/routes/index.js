var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express get' });
});

/* POST home page. */
router.post('/', function(req, res, next) {
  res.render('index', { title: 'Express post' });
});

module.exports = router;
