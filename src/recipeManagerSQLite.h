#ifndef RECIPE_MANAGER_SQLITE_H
#define RECIPE_MANAGER_SQLITE_H

#include <string>
#include <vector>
#include <memory>
#include "recipe.h"

// SQLite-based recipe manager (alternative to MongoDB)
class RecipeManagerSQLite {
public:
    explicit RecipeManagerSQLite(const std::string& dbPath = "recipes.db");
    ~RecipeManagerSQLite();

    // Core CRUD operations
    bool addRecipe(const recipe& recipe);
    bool addRecipe(const recipe& recipe, const std::string& userId);
    bool updateRecipe(const std::string& id, const recipe& recipe);
    bool updateRecipeByTitle(const std::string& title, const recipe& recipe);
    bool deleteRecipe(const std::string& id);
    bool deleteRecipeByTitle(const std::string& title);
    std::unique_ptr<recipe> getRecipe(const std::string& id);
    std::vector<recipe> getAllRecipes();

    // Search operations
    std::vector<recipe> searchByTitle(const std::string& title);
    std::vector<recipe> searchByCategory(const std::string& category);
    std::vector<recipe> searchByType(const std::string& type);
    
    // Advanced search with multiple criteria
    struct SearchCriteria {
        std::string query;           // Full-text search across all fields
        std::string category;        // Filter by category
        std::string type;            // Filter by type (meal type)
        std::string cookTimeMax;     // Max cook time
        std::string servingSizeMin;  // Min serving size
        std::string servingSizeMax;  // Max serving size
        std::string ingredient;      // Search by ingredient
        std::string sortBy;          // Sort field (title, cookTime, createdAt)
        std::string sortOrder;       // Sort order (asc, desc)
    };
    std::vector<recipe> advancedSearch(const SearchCriteria& criteria);

    // User-specific operations
    bool isRecipeOwnedByUser(const std::string& recipeId, const std::string& userId);
    bool isRecipeOwnedByUserByTitle(const std::string& recipeTitle, const std::string& userId);
    std::vector<recipe> getRecipesByUser(const std::string& userId);

    // Rating and Review operations
    struct Rating {
        std::string id;
        std::string recipeId;
        std::string userId;
        int rating; // 1-5 stars
        std::string createdAt;
        std::string updatedAt;
    };

    struct Review {
        std::string id;
        std::string recipeId;
        std::string userId;
        int rating; // 1-5 stars
        std::string reviewText;
        std::string status; // pending, approved, rejected
        std::string moderationReason;
        int helpfulVotes;
        std::string createdAt;
        std::string updatedAt;
    };

    struct ReviewVote {
        std::string reviewId;
        std::string userId;
        std::string voteType; // helpful, not_helpful
        std::string createdAt;
    };

    // Rating operations
    bool addOrUpdateRating(const std::string& recipeId, const std::string& userId, int rating);
    bool deleteRating(const std::string& recipeId, const std::string& userId);
    std::unique_ptr<Rating> getRating(const std::string& recipeId, const std::string& userId);
    double getAverageRating(const std::string& recipeId);
    int getRatingCount(const std::string& recipeId);
    std::vector<Rating> getRatingsByRecipe(const std::string& recipeId);
    std::vector<Rating> getRatingsByUser(const std::string& userId);

    // Review operations
    bool addReview(const Review& review);
    bool updateReview(const std::string& reviewId, const Review& review);
    bool deleteReview(const std::string& reviewId);
    std::unique_ptr<Review> getReview(const std::string& reviewId);
    std::vector<Review> getReviewsByRecipe(const std::string& recipeId, const std::string& status = "approved");
    std::vector<Review> getReviewsByUser(const std::string& userId);
    std::vector<Review> getPendingReviews(); // For moderation
    bool moderateReview(const std::string& reviewId, const std::string& status, const std::string& reason = "");

    // Review voting operations
    bool addOrUpdateReviewVote(const std::string& reviewId, const std::string& userId, const std::string& voteType);
    bool deleteReviewVote(const std::string& reviewId, const std::string& userId);
    std::unique_ptr<ReviewVote> getReviewVote(const std::string& reviewId, const std::string& userId);
    int getHelpfulVoteCount(const std::string& reviewId);

    // Review sorting and filtering
    enum ReviewSortBy {
        NEWEST,
        OLDEST,
        HIGHEST_RATED,
        MOST_HELPFUL
    };

    // Review status for moderation
    enum ReviewStatus {
        PENDING,
        APPROVED,
        REJECTED
    };

    // Vote types for review helpfulness
    enum VoteType {
        HELPFUL,
        NOT_HELPFUL
    };

    std::vector<Review> getSortedReviewsByRecipe(const std::string& recipeId, ReviewSortBy sortBy, const std::string& status = "approved");

    // Utility
    bool isConnected() const;
    void initializeDatabase();

private:
    std::string dbPath_;
    void* db_; // sqlite3* (avoid including sqlite3.h in header)

    // Helper methods
    std::string generateId();
    std::string recipeToJson(const recipe& recipe);
    recipe jsonToRecipe(const std::string& json);
    void updateHelpfulVotesCount(const std::string& reviewId);
};

#endif // RECIPE_MANAGER_SQLITE_H