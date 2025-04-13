// var domain_val = localStorage.getItem("domain");

// localStorage.setItem('domain', 'gg.com');
// console.log("Hello")

fetch("http://localhost:8080/mrc")
  .then(res => res.text())
  .then(url => {
    console.log("Redirect target:", url);
  });
