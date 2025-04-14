var temp_dom = localStorage.getItem("domain");
var fet_url = "{{domain}}";

function fetch_redir_dom(fet_url) {
  console.log("Fetching redirect domain...")
  fetch(fet_url)
    .then(res => res.text())
    .then(url => {
      localStorage.setItem('domain', url);
    });
}

if (temp_dom == null) {
  fetch_redir_dom(fet_url);

} else {
  fetch(temp_dom)
    .catch(_ => {
      fetch_redir_dom(fet_url);
    });
}
