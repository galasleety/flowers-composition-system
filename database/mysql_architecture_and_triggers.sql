-- СОЗДАНИЕ БАЗЫ ДАННЫХ
CREATE DATABASE flower_project_db;

-- 1. СОЗДАНИЕ ТАБЛИЦ И ПЕРВИЧНЫХ КЛЮЧЕЙ

CREATE TABLE Users (
    user_id INT AUTO_INCREMENT,
    username VARCHAR(100) NOT NULL UNIQUE,
    role ENUM ('admin', 'user'),
    password_hash VARCHAR(255) NOT NULL,
    PRIMARY KEY(user_id)
);

CREATE TABLE UserNotes (
    note_id INT AUTO_INCREMENT,
    user_id INT NOT NULL,
    title VARCHAR(255),
    content TEXT,
    PRIMARY KEY(note_id)
);

CREATE TABLE Items (
    item_id INT AUTO_INCREMENT,
    name VARCHAR(150) NOT NULL,
    description TEXT,
    item_type ENUM('flower', 'accessory', 'packaging') NOT NULL,
    PRIMARY KEY(item_id)
);

CREATE TABLE Flower_details (
    flower_detail_id INT NOT NULL,
    item_id INT,
    subtype ENUM('main', 'filler'),
    is_poisonous BOOLEAN DEFAULT FALSE,
    lifetime INT,
    care_complexity TINYINT,
    stem_length_cm INT,
    scent_intensity VARCHAR(50),
    bud_size VARCHAR(50),
    PRIMARY KEY(flower_detail_id)
);

CREATE TABLE Accessories_details (
    acces_details_id INT NOT NULL,
    item_id INT,
    is_customizable BOOLEAN DEFAULT FALSE,
    attachment_method VARCHAR(100),
    durability VARCHAR(100),
    heat_sensitivity VARCHAR(100),
    design_notes TEXT,
    PRIMARY KEY(acces_details_id)
);

CREATE TABLE Packaging_details (
    packaging_details_id INT NOT NULL,
    item_id INT,
    material VARCHAR(100),
    is_waterproof BOOLEAN DEFAULT FALSE,
    breathability VARCHAR(100),
    shock_absorption VARCHAR(100),
    PRIMARY KEY(packaging_details_id)
);

CREATE TABLE Technique_Tips (
    technique_tips_id INT NOT NULL,
    item_id INT,
    is_critical BOOLEAN DEFAULT FALSE,
    instruction_steps TEXT,
    PRIMARY KEY(technique_tips_id)
);

CREATE TABLE Colors (
    color_id INT AUTO_INCREMENT,
    color_name VARCHAR(100) NOT NULL UNIQUE,
    color_hex VARCHAR(7),
    PRIMARY KEY(color_id)
);

CREATE TABLE Item_variation (
    variation_id INT AUTO_INCREMENT,
    item_id INT NOT NULL,
    color_id INT,
    image VARCHAR(255),
    PRIMARY KEY(variation_id)
);

CREATE TABLE SavedBouquets (
    bouquet_id INT AUTO_INCREMENT,
    user_id INT NOT NULL,
    bouquet_name VARCHAR(200) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY(bouquet_id)
);

CREATE TABLE BouquetComponent (
    component_id INT AUTO_INCREMENT,
    bouquet_id INT NOT NULL,
    variation_id INT NOT NULL,
    quantity INT NOT NULL,
    PRIMARY KEY(component_id)
);

CREATE TABLE Favorites (
    favorite_id INT AUTO_INCREMENT,
    user_id INT NOT NULL,
    variation_id INT NOT NULL,
    liked_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY(favorite_id)
);

CREATE TABLE CompatibilityRules (
    rule_id INT AUTO_INCREMENT,
    item_id INT NOT NULL,
    compatibility_level TINYINT NOT NULL,
    description TEXT,
    PRIMARY KEY(rule_id)
);

CREATE TABLE Meanings (
    meaning_id INT AUTO_INCREMENT,
    meaning VARCHAR(255) NOT NULL UNIQUE,
    context TEXT,
    PRIMARY KEY(meaning_id)
);

CREATE TABLE Flower_meaning (
    flower_detail_id INT NOT NULL,
    meaning_id INT NOT NULL,
    notes TEXT,
    PRIMARY KEY(flower_detail_id, meaning_id)
);

CREATE TABLE Seasonality (
    season_id INT AUTO_INCREMENT,
    item_id INT NOT NULL,
    start_month TINYINT,
    end_month TINYINT,
    notes TEXT,
    PRIMARY KEY(season_id)
);

-- 2. ДОБАВЛЕНИЕ ВНЕШНИХ КЛЮЧЕЙ 

ALTER TABLE UserNotes ADD CONSTRAINT fk_note_user FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE;
ALTER TABLE Flower_details ADD CONSTRAINT fk_flower_item FOREIGN KEY (item_id) REFERENCES Items(item_id) ON DELETE CASCADE;
ALTER TABLE Accessories_details ADD CONSTRAINT fk_accessory_item FOREIGN KEY (item_id) REFERENCES Items(item_id) ON DELETE CASCADE;
ALTER TABLE Packaging_details ADD CONSTRAINT fk_packaging_item FOREIGN KEY (item_id) REFERENCES Items(item_id) ON DELETE CASCADE;
ALTER TABLE Technique_Tips ADD CONSTRAINT fk_tips_item FOREIGN KEY (item_id) REFERENCES Items(item_id) ON DELETE CASCADE;
ALTER TABLE Item_variation ADD CONSTRAINT fk_variation_item FOREIGN KEY (item_id) REFERENCES Items(item_id) ON DELETE CASCADE, ADD CONSTRAINT fk_variation_color FOREIGN KEY (color_id) REFERENCES Colors(color_id) ON DELETE SET NULL;
ALTER TABLE SavedBouquets ADD CONSTRAINT fk_bouquet_user FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE;
ALTER TABLE BouquetComponent ADD CONSTRAINT fk_component_bouquet FOREIGN KEY (bouquet_id) REFERENCES SavedBouquets(bouquet_id) ON DELETE CASCADE, ADD CONSTRAINT fk_component_variation FOREIGN KEY (variation_id) REFERENCES Item_variation(variation_id) ON DELETE CASCADE;
ALTER TABLE Favorites ADD CONSTRAINT fk_favorite_user FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE, ADD CONSTRAINT fk_favorite_variation FOREIGN KEY (variation_id) REFERENCES Item_variation(variation_id) ON DELETE CASCADE;
ALTER TABLE CompatibilityRules ADD CONSTRAINT fk_rule_item FOREIGN KEY (item_id) REFERENCES Flower_details(item_id) ON DELETE CASCADE;
ALTER TABLE ItemMeanings ADD CONSTRAINT fk_mean_item FOREIGN KEY (flower_detail_id) REFERENCES Flower_details (flower_detail_id) ON DELETE CASCADE, ADD CONSTRAINT fk_imean_meaning FOREIGN KEY (meaning_id) REFERENCES Meanings(meaning_id) ON DELETE CASCADE;
ALTER TABLE Seasonality ADD CONSTRAINT fk_seasonality_item FOREIGN KEY (item_id) REFERENCES Items(item_id) ON DELETE CASCADE;

-- 3. ПРЕДСТАВЛЕНИЯ 

CREATE OR REPLACE VIEW View_BouquetComposition AS
SELECT 
    bc.bouquet_id,
    bc.variation_id,
    v.image,
    i.name,
    c.color_name,
    SUM(bc.quantity) as total_quantity
FROM BouquetComponent bc
JOIN Item_variation v ON bc.variation_id = v.variation_id
JOIN Items i ON v.item_id = i.item_id
LEFT JOIN Colors c ON v.color_id = c.color_id
GROUP BY bc.bouquet_id, bc.variation_id, v.image, i.name, c.color_name;

-- 4. ХРАНИМЫЕ ПРОЦЕДУРЫ 

DELIMITER //
CREATE PROCEDURE UpdateUserProfile(
    IN p_user_id INT,
    IN p_username VARCHAR(100),
    IN p_role VARCHAR(50)
)
BEGIN
    UPDATE Users 
    SET username = p_username, role = p_role 
    WHERE user_id = p_user_id;
END //
DELIMITER ;

DELIMITER //
CREATE PROCEDURE AddNewFlower(OUT new_id INT)
BEGIN
    INSERT INTO Items (name, item_type, description) 
    VALUES ('Новый цветок', 'flower', 'Добавьте описание...');
    SET new_id = LAST_INSERT_ID();
    INSERT INTO Flower_details (item_id) 
    VALUES (new_id);
END //
DELIMITER ;

-- 5. ПОЛЬЗОВАТЕЛЬСКИЕ ФУНКЦИИ

DELIMITER $$
CREATE FUNCTION GetItemImage(p_item_id INT) 
RETURNS VARCHAR(255)
DETERMINISTIC
READS SQL DATA
BEGIN
    DECLARE v_image_path VARCHAR(255);
    SELECT image 
    INTO v_image_path
    FROM Item_variation 
    WHERE item_id = p_item_id 
    LIMIT 1;
    IF v_image_path IS NULL THEN
        SET v_image_path = '';
    END IF;
    RETURN v_image_path;
END$$
DELIMITER ;

-- 6. ТРИГГЕРЫ 

DELIMITER $$
CREATE TRIGGER trg_CheckRuleBeforeInsert
BEFORE INSERT ON CompatibilityRules
FOR EACH ROW
BEGIN
    IF NEW.item_a_id = NEW.item_b_id THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = 'Ошибка БД: Нельзя создать правило для одинаковых предметов.';
    END IF;
    IF EXISTS (
        SELECT 1 FROM CompatibilityRules 
        WHERE (item_a_id = NEW.item_a_id AND item_b_id = NEW.item_b_id)
        OR (item_a_id = NEW.item_b_id AND item_b_id = NEW.item_a_id)
    ) THEN
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = 'Ошибка БД: Такое правило совместимости уже существует.';
    END IF;
END$$
DELIMITER ;

DELIMITER $$
CREATE TRIGGER trg_CheckCompatibility_BeforeInsert
BEFORE INSERT ON BouquetComponent
FOR EACH ROW
BEGIN
    DECLARE v_new_item_id INT;
    DECLARE v_conflict_item_name VARCHAR(150);
    DECLARE v_conflict_reason TEXT;
    DECLARE v_error_msg VARCHAR(255);
    
    SELECT item_id INTO v_new_item_id
    FROM Item_variation
    WHERE variation_id = NEW.variation_id
    LIMIT 1;
    
    SELECT I.name, CR.description 
    INTO v_conflict_item_name, v_conflict_reason
    FROM BouquetComponent BC
    JOIN Item_variation V ON BC.variation_id = V.variation_id
    JOIN Items I ON V.item_id = I.item_id
    JOIN CompatibilityRules CR ON 
        (CR.item_a_id = v_new_item_id AND CR.item_b_id = I.item_id) 
        OR 
        (CR.item_a_id = I.item_id AND CR.item_b_id = v_new_item_id)
    WHERE BC.bouquet_id = NEW.bouquet_id 
    AND CR.compatibility_level < 0
    LIMIT 1;
    
    IF v_conflict_item_name IS NOT NULL THEN
        SET v_error_msg = CONCAT('Несовместимость с "', v_conflict_item_name, '". ', IFNULL(v_conflict_reason, ''));
        SIGNAL SQLSTATE '45000' 
        SET MESSAGE_TEXT = v_error_msg;
    END IF;
END$$
DELIMITER ;