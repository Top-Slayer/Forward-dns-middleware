fetch("http://localhost:8080/mrc").then(t=>t.text()).then(t=>{console.log("Redirect target:",t)});
