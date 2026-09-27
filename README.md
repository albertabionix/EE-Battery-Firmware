# EE-XX-Firmware

This repo holds the firmware for [brief description of team].

## Contributing Members

| First Name | Username | Project |
|---|---|---|
| Name | @user | task |
|  |  |  |
|  |  |  |

(Fill in your information in the above table if you contribute to this repo)

## Rules for Contributing

1. **Never push directly to `main`.** All changes go through a pull request.
2. **Create a new branch for your work.** Name it something short and clear, like `yourname-feature` (e.g. `tamu-BMS-address-fix`).
   ```
   git checkout -b yourname-feature
   ```
3. **Commit often, with clear messages.** A good commit message says *what* changed and *why*, not just "updated file."
4. **Open a pull request (PR) when your work is ready.** Describe what you did and why.
5. **Get at least 1 approval before merging.** Someone else on the team needs to review your PR first. 
6. **Don't force-push or delete branches** without checking with the team — this can erase other people's work.
7. **Keep PRs small and focused.** One feature or fix per PR is easier to review than one giant PR with everything in it.
8. If you're stuck or confused about something relating to Git or your work, **ask in the Discord server**!

## The Basics
 
New to git? Follow these steps to get set up. If you get stuck at any point, just ask an EE or SWE lead for help on the Discord server.
 
### 0. Create a GitHub account
Sign up at [github.com](https://github.com) if you haven't already.
 
### 1. Install Git
Download it from [git-scm.com](https://git-scm.com) and install with the default options. On Windows this also installs **Git Bash**, a terminal you can run git commands in. On Mac, you can also just type `git --version` in Terminal — it'll prompt you to install if it's missing.

### 2. Set your git identity
Open a terminal (or Git Bash) and run:
```bash
git config --global user.name "Your Name"
git config --global user.email "your@email.com"
```
This labels your future commits so teammates know who made them.
 
### 3. Check repo access
Ask an electrical or software lead to add your GitHub username as a collaborator. You won't be able to work until you're added.
 
### 4. Set up authentication
GitHub no longer accepts passwords for git operations. Easiest fix: install [GitHub Desktop](https://desktop.github.com) or the [GitHub CLI](https://cli.github.com) and sign in through the browser prompt — it handles authentication for you.
 
### 5. Clone the repo
```bash
git clone <repo-url>
cd EE-XX-Firmware
```
Copy `<repo-url>` from the green **Code** button on the GitHub repo page.
 
### 6. Create your branch and start working
```bash
git checkout -b yourname/your-feature-name
```
Make your changes, then:
```bash
git add .
git commit -m "short description of what you did"
git push -u origin yourname/your-feature-name
```
Open a pull request on GitHub for review and merge. One of the leads will review it and approve your work; then you can merge it.

## TLDR :D
Once you have Git/Github all set up. Create a new branch, then make your changes, commit them, push your branch, and open a PR on GitHub.

