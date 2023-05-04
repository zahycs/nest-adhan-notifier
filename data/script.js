(function (doc, win) {

  const tabs = document.querySelectorAll(".tabs-list-item");
  const tabsContent = document.querySelectorAll(".tabs-content");
  tabs.forEach((tab) => {
    tab.addEventListener("click", (e) => {
      const activeTab = doc.getElementById(e.currentTarget.dataset.contentId);
      tabs.forEach((tab) => tab.classList.remove("active"));
      tabsContent.forEach((tab) => tab.classList.remove("active"));
      tab.classList.add("active");
      activeTab.classList.add("active");
    })
  });
})(document, window);
